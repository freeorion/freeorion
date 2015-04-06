import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import TechsListsAI
import AIDependencies
import AIstate
import traceback
import ColonisationAI
from freeorion_tools import tech_is_complete, chat_human

inProgressTechs = {}


def get_research_index():
    empire_id = fo.empireID()
    research_index = empire_id % 2
    if foAI.foAIstate.aggression >= fo.aggression.aggressive:  # maniacal
        research_index = 2 + (empire_id % 3)  # so indices [2,3,4]
    elif foAI.foAIstate.aggression >= fo.aggression.typical:
        research_index += 1
    return research_index


def exclude_tech(tech_name):
    return ((foAI.foAIstate.aggression < AIDependencies.TECH_EXCLUSION_MAP_1.get(tech_name, fo.aggression.invalid)) or
            (foAI.foAIstate.aggression > AIDependencies.TECH_EXCLUSION_MAP_2.get(tech_name, fo.aggression.maniacal)) or
            tech_name in TechsListsAI.unusable_techs())
            

def generate_research_orders():
    """generate research orders"""
    report_adjustments = False
    empire = fo.getEmpire()
    empire_id = empire.empireID
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted', {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    print "Research Queue Management:"
    resource_production = empire.resourceProduction(fo.resourceType.research)
    print "\nTotal Current Research Points: %.2f\n" % resource_production
    print "Techs researched and available for use:"
    completed_techs = sorted(list(get_completed_techs()))
    tlist = completed_techs+3*[" "]
    tlines = zip(tlist[0::3], tlist[1::3], tlist[2::3])
    for tline in tlines:
        print "%25s %25s %25s" % tline
    print
    
    #
    # report techs currently at head of research queue
    #
    research_queue = empire.researchQueue
    research_queue_list = get_research_queue_techs()
    inProgressTechs.clear()
    tech_turns_left = {}
    if research_queue_list:
        print "Techs currently at head of Research Queue:"
        for element in list(research_queue)[:10]:
            tech_turns_left[element.tech] = element.turnsLeft
            if element.allocation > 0.0:
                inProgressTechs[element.tech] = True
            this_tech = fo.getTech(element.tech)
            if not this_tech:
                print "Error: can't retrieve tech ", element.tech
                continue
            missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in completed_techs]
            # unlocked_items = [(uli.name, uli.type) for uli in this_tech.unlocked_items]
            unlocked_items = [uli.name for uli in this_tech.unlockedItems]
            if not missing_prereqs:
                print "    %25s allocated %6.2f RP -- unlockable items: %s " % (element.tech, element.allocation, unlocked_items)
            else:
                print "    %25s allocated %6.2f RP -- missing preReqs: %s -- unlockable items: %s " % (element.tech, element.allocation, missing_prereqs, unlocked_items)
        print
    #
    # set starting techs, or after turn 100 add any additional default techs
    #
    if (fo.currentTurn() == 1) or ((fo.currentTurn() < 5) and (len(research_queue_list) == 0)):
        research_index = get_research_index()
        new_tech = TechsListsAI.sparse_galaxy_techs(research_index) if galaxy_is_sparse else TechsListsAI.primary_meta_techs(research_index)
        print "Empire %s (%d) is selecting research index %d" % (empire.name, empire_id, research_index)
        # techs_to_enqueue = (set(new_tech)-(set(completed_techs)|set(research_queue_list)))
        techs_to_enqueue = new_tech[:]
        tech_base = set(completed_techs+research_queue_list)
        techs_to_add = []
        for tech in techs_to_enqueue:
            if tech not in tech_base:
                this_tech = fo.getTech(tech)
                if this_tech is None:
                    print "Error: desired tech '%s' appears to not exist" % tech
                    continue
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq not in tech_base]
                techs_to_add.extend(missing_prereqs + [tech])
                tech_base.update(missing_prereqs+[tech])
        cum_cost = 0
        print "  Enqueued Tech: %20s \t\t %8s \t %s" % ("Name", "Cost", "CumulativeCost")
        for name in techs_to_add:
            try:
                enqueue_res = fo.issueEnqueueTechOrder(name, -1)
                if enqueue_res == 1:
                    this_tech = fo.getTech(name)
                    this_cost = 0
                    if this_tech:
                        this_cost = this_tech.researchCost(empire_id)
                        cum_cost += this_cost
                    print "    Enqueued Tech: %20s \t\t %8.0f \t %8.0f" % (name, this_cost, cum_cost)
                else:
                    print "    Error: failed attempt to enqueued Tech: " + name
            except:
                print "    Error: failed attempt to enqueued Tech: " + name
                print "    Error: exception triggered and caught: ", traceback.format_exc()
        if foAI.foAIstate.aggression <= fo.aggression.cautious:
            research_queue_list = get_research_queue_techs()
            def_techs = TechsListsAI.defense_techs_1()
            for def_tech in def_techs:
                if def_tech not in research_queue_list[:5] and not tech_is_complete(def_tech):
                    res = fo.issueEnqueueTechOrder(def_tech, min(3, len(research_queue_list)))
                    print "Empire is very defensive, so attempted to fast-track %s, got result %d" % (def_tech, res)
        if False and foAI.foAIstate.aggression >= fo.aggression.aggressive:  # with current stats of Conc Camps, disabling this fast-track
            research_queue_list = get_research_queue_techs()
            if "CON_CONC_CAMP" in research_queue_list:
                insert_idx = min(40, research_queue_list.index("CON_CONC_CAMP"))
            else:
                insert_idx = max(0, min(40, len(research_queue_list)-10))
            if "SHP_DEFLECTOR_SHIELD" in research_queue_list:
                insert_idx = min(insert_idx, research_queue_list.index("SHP_DEFLECTOR_SHIELD"))
            for cc_tech in ["CON_ARCH_PSYCH", "CON_CONC_CAMP"]:
                if cc_tech not in research_queue_list[:insert_idx + 1] and not tech_is_complete(cc_tech):
                    res = fo.issueEnqueueTechOrder(cc_tech, insert_idx)
                    msg = "Empire is very aggressive, so attempted to fast-track %s, got result %d" % (cc_tech, res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg

        print""

        generate_default_research_order()
        print "\n\nAll techs:"
        alltechs = fo.techs()  # returns names of all techs
        for tname in alltechs:
            print tname
        print "\n-------------------------------\nAll unqueued techs:"
        # coveredTechs = new_tech+completed_techs
        for tname in [tn for tn in alltechs if tn not in tech_base]:
            print tname

    elif fo.currentTurn() > 100:
        generate_default_research_order()

    research_queue_list = get_research_queue_techs()
    num_techs_accelerated = 1  # will ensure leading tech doesn't get dislodged
    got_ggg_tech = tech_is_complete("PRO_ORBITAL_GEN")
    got_sym_bio = tech_is_complete("GRO_SYMBIOTIC_BIO")
    got_xeno_gen = tech_is_complete("GRO_XENO_GENETICS")
    #
    # Consider accelerating techs; priority is
    # Supply/Detect range
    # xeno arch
    # ast / GG
    # gro xeno gen
    # distrib thought
    # quant net
    # pro sing gen
    # death ray 1 cleanup

    #
    # Supply range and detection range
    if False:  # disabled for now, otherwise just to help with cold-folding / organization
        if len(foAI.foAIstate.colonisablePlanetIDs) == 0:
            best_colony_site_score = 0
        else:
            best_colony_site_score = foAI.foAIstate.colonisablePlanetIDs.items()[0][1]
        if len(foAI.foAIstate.colonisableOutpostIDs) == 0:
            best_outpost_site_score = 0
        else:
            best_outpost_site_score = foAI.foAIstate.colonisableOutpostIDs.items()[0][1]
        need_improved_scouting = (best_colony_site_score < 150 or best_outpost_site_score < 200)

        if need_improved_scouting:
            if not tech_is_complete("CON_ORBITAL_CON"):
                num_techs_accelerated += 1
                if ("CON_ORBITAL_CON" not in research_queue_list[:1 + num_techs_accelerated]) and (
                        tech_is_complete("PRO_FUSION_GEN") or ("PRO_FUSION_GEN" in research_queue_list[:1 + num_techs_accelerated])):
                    res = fo.issueEnqueueTechOrder("CON_ORBITAL_CON", num_techs_accelerated)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_ORBITAL_CON", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
            elif not tech_is_complete("CON_CONTGRAV_ARCH"):
                num_techs_accelerated += 1
                if ("CON_CONTGRAV_ARCH" not in research_queue_list[:1+num_techs_accelerated]) and (
                        tech_is_complete("CON_METRO_INFRA")):
                    for supply_tech in [_s_tech for _s_tech in ["CON_ARCH_MONOFILS", "CON_CONTGRAV_ARCH"] if not tech_is_complete(_s_tech)]:
                        res = fo.issueEnqueueTechOrder(supply_tech, num_techs_accelerated)
                        msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % (supply_tech, res)
                        if report_adjustments:
                            chat_human(msg)
                        else:
                            print msg
            elif not tech_is_complete("CON_GAL_INFRA"):
                num_techs_accelerated += 1
                if ("CON_GAL_INFRA" not in research_queue_list[:1+num_techs_accelerated]) and (
                        tech_is_complete("PRO_SINGULAR_GEN")):
                    res = fo.issueEnqueueTechOrder("CON_GAL_INFRA", num_techs_accelerated)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_GAL_INFRA", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
            else:
                pass
            research_queue_list = get_research_queue_techs()
            # could add more supply tech

            if False and not tech_is_complete("SPY_DETECT_2"):  # disabled for now, detect2
                num_techs_accelerated += 1
                if "SPY_DETECT_2" not in research_queue_list[:2+num_techs_accelerated] and tech_is_complete("PRO_FUSION_GEN"):
                    if "CON_ORBITAL_CON" not in research_queue_list[:1+num_techs_accelerated]:
                        res = fo.issueEnqueueTechOrder("SPY_DETECT_2", num_techs_accelerated)
                    else:
                        co_idx = research_queue_list.index("CON_ORBITAL_CON")
                        res = fo.issueEnqueueTechOrder("SPY_DETECT_2", co_idx + 1)
                    msg = "Empire has poor colony/outpost prospects, so attempted to fast-track %s, got result %d" % ("CON_ORBITAL_CON", res)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
                research_queue_list = get_research_queue_techs()

    #
    # check to accelerate xeno_arch
    if True:  # just to help with cold-folding /  organization
        if (ColonisationAI.gotRuins and not tech_is_complete("LRN_XENOARCH") and
                foAI.foAIstate.aggression >= fo.aggression.typical):
            if "LRN_ARTIF_MINDS" in research_queue_list:
                insert_idx = 7 + research_queue_list.index("LRN_ARTIF_MINDS")
            elif "GRO_SYMBIOTIC_BIO" in research_queue_list:
                insert_idx = research_queue_list.index("GRO_SYMBIOTIC_BIO") + 1
            else:
                insert_idx = num_techs_accelerated
            if "LRN_XENOARCH" not in research_queue_list[:insert_idx]:
                for xenoTech in ["LRN_XENOARCH", "LRN_TRANSLING_THT", "LRN_PHYS_BRAIN", "LRN_ALGO_ELEGANCE"]:
                    if not tech_is_complete(xenoTech) and xenoTech not in research_queue_list[:(insert_idx + 4)]:
                        res = fo.issueEnqueueTechOrder(xenoTech, insert_idx)
                        num_techs_accelerated += 1
                        msg = "ANCIENT_RUINS: have an ancient ruins, so attempted to fast-track %s to enable LRN_XENOARCH, got result %d" % (xenoTech, res)
                        if report_adjustments:
                            chat_human(msg)
                        else:
                            print msg
                research_queue_list = get_research_queue_techs()

    if False and not enemies_sighted:  # curently disabled
        # params = [ (tech, gate, target_slot, add_tech_list), ]
        params = [("GRO_XENO_GENETICS", "PRO_EXOBOTS", "PRO_EXOBOTS", ["GRO_GENETIC_MED", "GRO_XENO_GENETICS"]),
                  ("PRO_EXOBOTS", "PRO_SENTIENT_AUTOMATION", "PRO_SENTIENT_AUTOMATION", ["PRO_EXOBOTS"]),
                  ("PRO_SENTIENT_AUTOMATION", "PRO_NANOTECH_PROD", "PRO_NANOTECH_PROD", ["PRO_SENTIENT_AUTOMATION"]),
                  ("PRO_INDUSTRY_CENTER_I", "GRO_SYMBIOTIC_BIO", "GRO_SYMBIOTIC_BIO", ["PRO_ROBOTIC_PROD", "PRO_FUSION_GEN", "PRO_INDUSTRY_CENTER_I"]),
                  ("GRO_SYMBIOTIC_BIO", "SHP_ORG_HULL", "SHP_ZORTRIUM_PLATE", ["GRO_SYMBIOTIC_BIO"]),
                  ]
        for (tech, gate, target_slot, add_tech_list) in params:
            if tech_is_complete(tech):
                break
            if tech_turns_left.get(gate, 0) not in [0, 1, 2]:  # needs to exclude -1, the flag for no predicted completion
                continue
            if target_slot in research_queue_list:
                target_index = 1 + research_queue_list.index(target_slot)
            else:
                target_index = num_techs_accelerated
            for move_tech in add_tech_list:
                print "for tech %s, target_slot %s, target_index:%s ; num_techs_accelerated:%s" % (move_tech, target_slot, target_index, num_techs_accelerated)
                if tech_is_complete(move_tech):
                    continue
                if target_index <= num_techs_accelerated:
                    num_techs_accelerated += 1
                if move_tech not in research_queue_list[:1 + target_index]:
                    res = fo.issueEnqueueTechOrder(move_tech, target_index)
                    msg = "Research: To prioritize %s, have advanced %s to slot %d" % (tech, move_tech, target_index)
                    if report_adjustments:
                        chat_human(msg)
                    else:
                        print msg
                    target_index += 1
    #
    # check to accelerate asteroid or GG tech
    if True:  # just to help with cold-folding / organization
        if ColonisationAI.got_ast:
            insert_idx = num_techs_accelerated if "GRO_SYMBIOTIC_BIO" not in research_queue_list else research_queue_list.index("GRO_SYMBIOTIC_BIO")
            ast_tech = "PRO_MICROGRAV_MAN"
            if not (tech_is_complete(ast_tech) or ast_tech in research_queue_list[:(1 + insert_idx)]):
                res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                num_techs_accelerated += 1
                msg = "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d" % (ast_tech, res)
                if report_adjustments:
                    chat_human(msg)
                else:
                    print msg
                research_queue_list = get_research_queue_techs()
            elif tech_is_complete("SHP_ZORTRIUM_PLATE"):
                insert_idx = (1 + insert_idx) if "LRN_FORCE_FIELD" not in research_queue_list else max(1 + insert_idx, research_queue_list.index("LRN_FORCE_FIELD") - 1)
                for ast_tech in ["SHP_ASTEROID_HULLS", "SHP_IMPROVED_ENGINE_COUPLINGS"]:
                    if not tech_is_complete(ast_tech) and ast_tech not in research_queue_list[:insert_idx + 1]:
                        res = fo.issueEnqueueTechOrder(ast_tech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        msg = "Asteroids: plan to colonize an asteroid belt, so attempted to fast-track %s , got result %d" % (ast_tech, res)
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
        if ColonisationAI.got_gg and not tech_is_complete("PRO_ORBITAL_GEN"):
            fusion_idx = 0 if "PRO_FUSION_GEN" not in research_queue_list else (1 + research_queue_list.index("PRO_FUSION_GEN"))
            forcefields_idx = 0 if "LRN_FORCE_FIELD" not in research_queue_list else (1 + research_queue_list.index("LRN_FORCE_FIELD"))
            insert_idx = max(fusion_idx, forcefields_idx) if enemies_sighted else fusion_idx
            if "PRO_ORBITAL_GEN" not in research_queue_list[:insert_idx+1]:
                res = fo.issueEnqueueTechOrder("PRO_ORBITAL_GEN", insert_idx)
                num_techs_accelerated += 1
                msg = "GasGiant: plan to colonize a gas giant, so attempted to fast-track %s, got result %d" % ("PRO_ORBITAL_GEN", res)
                print msg
                if report_adjustments:
                    chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # assess if our empire has any non-lousy colonizers, & boost gro_xeno_gen if we don't
    if True:  # just to help with cold-folding / organization
        if got_ggg_tech and got_sym_bio and (not got_xeno_gen) and foAI.foAIstate.aggression >= fo.aggression.cautious:
            most_adequate = 0
            for specName in ColonisationAI.empire_colonizers:
                environs = {}
                this_spec = fo.getSpecies(specName)
                if not this_spec:
                    continue
                for ptype in [fo.planetType.swamp, fo.planetType.radiated, fo.planetType.toxic, fo.planetType.inferno, fo.planetType.barren, fo.planetType.tundra, fo.planetType.desert, fo.planetType.terran, fo.planetType.ocean, fo.planetType.asteroids]:
                    environ = this_spec.getPlanetEnvironment(ptype)
                    environs.setdefault(environ, []).append(ptype)
                most_adequate = max(most_adequate, len(environs.get(fo.planetEnvironment.adequate, [])))
            if most_adequate == 0:
                insert_idx = num_techs_accelerated
                for xg_tech in ["GRO_XENO_GENETICS", "GRO_GENETIC_ENG"]:
                    if xg_tech not in research_queue_list[:1+num_techs_accelerated] and not tech_is_complete(xg_tech):
                        res = fo.issueEnqueueTechOrder(xg_tech, insert_idx)
                        num_techs_accelerated += 1
                        msg = "Empire has poor colonizers, so attempted to fast-track %s, got result %d" % (xg_tech, res)
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate distrib thought
    if True:  # just to help with cold-folding / organization
        if not tech_is_complete("LRN_DISTRIB_THOUGHT"):
            got_telepathy = False
            for specName in ColonisationAI.empire_species:
                this_spec = fo.getSpecies(specName)
                if this_spec and ("TELEPATHIC" in list(this_spec.tags)):
                    got_telepathy = True
                    break
            if (foAI.foAIstate.aggression > fo.aggression.cautious) and (empire.population() > ([300, 100][got_telepathy])):
                insert_idx = num_techs_accelerated
                for dt_ech in ["LRN_PHYS_BRAIN", "LRN_TRANSLING_THT", "LRN_PSIONICS", "LRN_DISTRIB_THOUGHT"]:
                    if dt_ech not in research_queue_list[:insert_idx + 2] and not tech_is_complete(dt_ech):
                        res = fo.issueEnqueueTechOrder(dt_ech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        fmt_str = "Empire has a telepathic race, so attempted to fast-track %s (got result %d)"
                        fmt_str += " with current target_RP %.1f and current pop %.1f, on turn %d"
                        msg = fmt_str % (dt_ech, res, resource_production, empire.population(), fo.currentTurn())
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()
    #
    # check to accelerate quant net
    if False:  # disabled for now, otherwise just to help with cold-folding / organization
        if (foAI.foAIstate.aggression > fo.aggression.cautious) and (ColonisationAI.empire_status.get('researchers', 0) >= 40):
            if not tech_is_complete("LRN_QUANT_NET"):
                insert_idx = num_techs_accelerated  # TODO determine min target slot if reenabling
                for qnTech in ["LRN_NDIM_SUBSPACE", "LRN_QUANT_NET"]:
                    if qnTech not in research_queue_list[:insert_idx + 2] and not tech_is_complete(qnTech):
                        res = fo.issueEnqueueTechOrder(qnTech, insert_idx)
                        num_techs_accelerated += 1
                        insert_idx += 1
                        msg = "Empire has many researchers, so attempted to fast-track %s (got result %d) on turn %d" % (qnTech, res, fo.currentTurn())
                        print msg
                        if report_adjustments:
                            chat_human(msg)
                research_queue_list = get_research_queue_techs()

    #
    # if we own a blackhole, accelerate sing_gen and conc camp
    if True:  # just to help with cold-folding / organization
        if (fo.currentTurn() > 50 and len(AIstate.empireStars.get(fo.starType.blackHole, [])) != 0 and
                foAI.foAIstate.aggression > fo.aggression.cautious and not tech_is_complete(AIDependencies.PRO_SINGULAR_GEN) and
                tech_is_complete(AIDependencies.PRO_SOL_ORB_GEN)):
            # sing_tech_list = [ "LRN_GRAVITONICS" , "PRO_SINGULAR_GEN"]  # formerly also "CON_ARCH_PSYCH", "CON_CONC_CAMP",
            sing_gen_tech = fo.getTech(AIDependencies.PRO_SINGULAR_GEN)
            sing_tech_list = [pre_req for pre_req in sing_gen_tech.recursivePrerequisites(empire_id) if not tech_is_complete(pre_req)]
            sing_tech_list += [AIDependencies.PRO_SINGULAR_GEN]
            for singTech in sing_tech_list:
                if singTech not in research_queue_list[:num_techs_accelerated+1]:
                    res = fo.issueEnqueueTechOrder(singTech, num_techs_accelerated)
                    num_techs_accelerated += 1
                    msg = "have a black hole star outpost/colony, so attempted to fast-track %s, got result %d" % (singTech, res)
                    print msg
                    if report_adjustments:
                        chat_human(msg)
            research_queue_list = get_research_queue_techs()

    #
    # if got deathray from Ruins, remove most prereqs from queue
    if True:  # just to help with cold-folding / organization
        if tech_is_complete("SHP_WEAPON_4_1"):
            this_tech = fo.getTech("SHP_WEAPON_4_1")
            if this_tech:
                missing_prereqs = [preReq for preReq in this_tech.recursivePrerequisites(empire_id) if preReq in research_queue_list]
                if len(missing_prereqs) > 2:  # leave plasma 4 and 3 if up to them already
                    for preReq in missing_prereqs:  # sorted(missing_prereqs, reverse=True)[2:]
                        if preReq in research_queue_list:
                            fo.issueDequeueTechOrder(preReq)
                    research_queue_list = get_research_queue_techs()
                    if "SHP_WEAPON_4_2" in research_queue_list:  # (should be)
                        idx = research_queue_list.index("SHP_WEAPON_4_2")
                        fo.issueEnqueueTechOrder("SHP_WEAPON_4_2", max(0, idx-18))


def generate_default_research_order():
    """
    Generate default research orders.
    Add cheapest technology from possible researches
    until current turn point totally spent.
    """

    empire = fo.getEmpire()
    total_rp = empire.resourceProduction(fo.resourceType.research)

    # get all usable researchable techs not already completed or queued for research

    queued_techs = get_research_queue_techs()

    def is_possible(tech_name):
        return all([empire.getTechStatus(tech_name) == fo.techStatus.researchable,
                   not tech_is_complete(tech_name),
                   not exclude_tech(tech_name),
                   tech_name not in queued_techs])

    # (cost, name) for all tech that possible to add to queue, cheapest last
    possible = sorted(
        [(fo.getTech(tech).researchCost(empire.empireID), tech) for tech in fo.techs() if is_possible(tech)],
        reverse=True)

    print "Techs in possible list after enqueues to Research Queue:"
    for _, tech in possible:
        print "    " + tech
    print

    # iterate through techs in order of cost
    fo.updateResearchQueue()
    total_spent = fo.getEmpire().researchQueue.totalSpent
    print "enqueuing techs. already spent RP: %s total RP: %s" % (total_spent, total_rp)

    while total_rp > 0 and possible:
        cost, name = possible.pop()  # get chipest
        total_rp -= cost
        fo.issueEnqueueTechOrder(name, -1)
        print "    enqueued tech " + name + "  : cost: " + str(cost) + "RP"
    print


def get_possible_projects():
    """get possible projects"""
    preliminary_projects = []
    empire = fo.getEmpire()
    for tech_name in fo.techs():
        if empire.getTechStatus(tech_name) == fo.techStatus.researchable:
            preliminary_projects.append(tech_name)
    return set(preliminary_projects)-set(TechsListsAI.unusable_techs())


def get_completed_techs():
    """get completed and available for use techs"""
    return [tech for tech in fo.techs() if tech_is_complete(tech)]


def get_research_queue_techs():
    """ Get list of techs in research queue."""
    return [element.tech for element in fo.getEmpire().researchQueue]

