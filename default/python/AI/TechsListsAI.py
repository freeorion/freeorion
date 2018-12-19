"""
The TechsListAI module provides functions that describes dependencies between
various technologies to help the AI decide which technologies should be
researched next.
"""
from logging import warn, debug

import freeOrionAIInterface as fo  # pylint: disable=import-error


def unusable_techs():
    """
    Returns a list of technologies that shouldn't be researched by the AI.
    """
    return []


def defense_techs_1():
    """
    Returns a list of technologies that implement the first planetary defensive
    measures.
    """
    return [
        "DEF_DEFENSE_NET_1",
        "DEF_GARRISON_1"
    ]


class TechGroup(object):
    """ Base class for Tech groups.

    A TechGroup consists of some techs which need to be researched before progressing to the next TechGroup.
    The techs are split into different categories, techs within each category are researched according to order.
    Different TechGroup variants have different orders in which the tech groups are accessed.
    """
    def __init__(self):
        self.economy = []     # list of economy techs to research
        self.weapon = []      # ship weapon techs
        self.armor = []       # ship armour techs
        self.misc = []        # techs not in any other category
        self.defense = []     # planetary defense techs
        self.hull = []        # new hull types

        self._tech_queue = []    # defines the research order - forced to contain all techs.
        self._errors = []          # exceptions that occured when trying to pop from already empty lists

    def _add_remaining(self):
        """Add all remaining techs in the tech group to self._tech_queue if not already contained."""
        all_needed_techs = set(self.economy + self.weapon + self.armor
                               + self.misc + self.defense + self.hull)
        remaining_techs = list(all_needed_techs - set(self._tech_queue))
        self._tech_queue.extend(remaining_techs)

    def get_techs(self):
        """Get the ordered list of techs defining research order.

        :return: Research order
        :rtype: list
        """
        self._add_remaining()
        return list(self._tech_queue)

    def enqueue(self, *tech_lists):
        """
        Pop first entry in the list or take entry if it is string and add it to research orders.

        Note that the passed list is modified within this function!
        If the list is already empty, the exception is caught and stored in self.__errors.
        Errors may be queried via get_errors()

        :type tech_lists: list[list | str]
        """

        for this_list in tech_lists:
            if isinstance(this_list, str):
                tech_name = this_list
            else:

                try:
                    tech_name = this_list.pop(0)
                except IndexError:
                    # Do not display error message as those should be shown only once per game session
                    # by the initial test_tech_integrity() call.
                    msg = "Try to enqueue tech from empty list"
                    warn(msg)
                    self._errors.append(msg)
                    continue
            if tech_name in self._tech_queue:
                msg = "Tech is already in queue: %s" % tech_name
                warn(msg)
                self._errors.append(msg)
            else:
                self._tech_queue.append(tech_name)

    def get_errors(self):
        """Return a list of occured exceptions.

        :rtype: list[Exception]
        """
        retval = list(self._errors)
        self._errors = []
        return retval


class TechGroup1(TechGroup):
    def __init__(self):
        super(TechGroup1, self).__init__()
        self.economy.extend([
            "LRN_PHYS_BRAIN",
            "GRO_PLANET_ECOL",
            "LRN_ALGO_ELEGANCE",
            "GRO_SUBTER_HAB",
            "LRN_ARTIF_MINDS",
            "PRO_ROBOTIC_PROD",
        ])
        self.weapon.extend([
            "SHP_WEAPON_1_2",
            "SHP_WEAPON_1_3",
            "SHP_FIGHTERS_1",
            "SHP_WEAPON_1_4",
        ])
        self.defense.extend([
            "DEF_GARRISON_1",
            "DEF_DEFENSE_NET_1",
        ])
        self.hull.extend([
            "SHP_MIL_ROBO_CONT",
            "SHP_ORG_HULL",
        ])
        # always start with the same first 8 techs; leaves 1 econ, 3 weap, 2 hull
        self.enqueue(
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.weapon,
            self.defense,
            self.defense,
        )


class TechGroup1a(TechGroup1):
    def __init__(self):
        super(TechGroup1a, self).__init__()
        self.enqueue(
            self.weapon,
            self.weapon,
            self.weapon,
            self.economy,
            self.hull,
        )


class TechGroup1b(TechGroup1):
    def __init__(self):
        super(TechGroup1b, self).__init__()
        self.enqueue(
            self.weapon,
            self.hull,
            self.economy,
            self.weapon,
            self.weapon,
        )


class TechGroup1SparseA(TechGroup1):
    def __init__(self):
        super(TechGroup1SparseA, self).__init__()
        self.enqueue(
            self.economy,
            self.hull,
            self.weapon,
            self.weapon,
            self.weapon,

            "SHP_SPACE_FLUX_DRIVE"
        )


class TechGroup1SparseB(TechGroup1):
    def __init__(self):
        super(TechGroup1SparseB, self).__init__()
        self.enqueue(
            self.economy,
            self.economy,
            self.weapon,
            self.hull,
            "SHP_ZORTRIUM_PLATE",
            self.weapon,
            "PRO_NANOTECH_PROD",
            "PRO_SENTIENT_AUTOMATION",
            "PRO_EXOBOTS",
            "CON_ORBITAL_CON",  # not a economy tech in the strictest sense but bonus supply often equals more planets
            "GRO_GENETIC_MED",
            "GRO_SYMBIOTIC_BIO",
            "PRO_MICROGRAV_MAN",
            "GRO_XENO_GENETICS",
            "LRN_FORCE_FIELD",
            "SHP_ASTEROID_HULLS",
            "PRO_FUSION_GEN",
            "SHP_WEAPON_2_1",
            self.hull,
            "SHP_MULTICELL_CAST",
            "SHP_WEAPON_2_2",
            "PRO_ORBITAL_GEN",
            "SPY_DETECT_2",
            "SHP_SPACE_FLUX_DRIVE",
        )
        self.weapon = []


class TechGroup1SparseC(TechGroup1):
    def __init__(self):
        super(TechGroup1SparseC, self).__init__()
        self.enqueue(
            self.economy,
            self.economy,
            self.hull,
            "SHP_MULTICELL_CAST",
            "PRO_NANOTECH_PROD",
            "PRO_SENTIENT_AUTOMATION",
            self.weapon,
            "CON_ORBITAL_CON",  # not a economy tech in the strictest sense but bonus supply often equals more planets
            "GRO_GENETIC_MED",
            "GRO_SYMBIOTIC_BIO",
            "PRO_EXOBOTS",
            "PRO_MICROGRAV_MAN",
            "GRO_XENO_GENETICS",
            "SHP_ASTEROID_HULLS",
            "PRO_FUSION_GEN",
            "SHP_WEAPON_2_1",
            "SHP_ZORTRIUM_PLATE",
            self.hull,
            "LRN_FORCE_FIELD",
            "SHP_WEAPON_2_2",
            "PRO_ORBITAL_GEN",
            "SPY_DETECT_2",
            "SHP_SPACE_FLUX_DRIVE",
        )
        self.weapon = []


class TechGroup2(TechGroup):
    def __init__(self):
        super(TechGroup2, self).__init__()
        self.economy.extend([
            "PRO_FUSION_GEN",
            "PRO_SENTIENT_AUTOMATION",
            "PRO_EXOBOTS",
            "GRO_SYMBIOTIC_BIO",
            "CON_ORBITAL_CON",  # not a economy tech in the strictest sense but bonus supply often equals more planets
            # "PRO_MICROGRAV_MAN",  # handled by fast-forwarding when we have asteroids
            # "PRO_ORBITAL_GEN",    # handled by fast-forwarding when we have a GG

        ])
        self.armor.extend([
            "SHP_ZORTRIUM_PLATE",
        ])
        self.hull.extend([
            "SHP_MULTICELL_CAST",
            "SHP_SPACE_FLUX_DRIVE",
            # "SHP_ASTEROID_HULLS",  # should be handled by fast-forwarding when having ASteroids
            # "SHP_DOMESTIC_MONSTER",  # should be handled by fast-forwarding when having nest
        ])
        self.weapon.extend([
            "SHP_WEAPON_2_1",
            "SHP_WEAPON_2_2",  # enables the building of laser based ships in ShipDesigner algorithm
            "SHP_FIGHTERS_2",
            "SHP_WEAPON_2_3",
            "SHP_WEAPON_2_4",
        ])


class TechGroup2A(TechGroup2):
    def __init__(self):
        super(TechGroup2A, self).__init__()
        self.defense.extend([
            "SPY_DETECT_2",
            "DEF_GARRISON_2",
            "LRN_FORCE_FIELD",
        ])
        self.enqueue(
            self.armor,
            self.economy,
            self.economy,
            self.economy,
            self.defense,
            self.defense,
            self.hull,
            self.weapon,
            self.weapon,
            self.defense,
            self.economy,
            self.economy,
            self.weapon,
            self.weapon,
            self.weapon,
            self.economy,
            self.hull,
        )


class TechGroup2B(TechGroup2):
    def __init__(self):
        super(TechGroup2B, self).__init__()
        self.defense.extend([
            "LRN_FORCE_FIELD",
            "SPY_DETECT_2",
            "DEF_GARRISON_2",
        ])
        self.enqueue(
            self.armor,
            self.economy,
            self.economy,
            self.defense,
            self.hull,
            self.weapon,
            self.weapon,
            self.economy,
            self.defense,
            self.defense,
            self.weapon,
            self.weapon,
            self.weapon,
            self.economy,
            self.economy,
            self.hull,
        )


class TechGroup2SparseA(TechGroup2):
    def __init__(self):
        super(TechGroup2SparseA, self).__init__()
        self.enqueue(
            self.armor,
            self.hull,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.defense,
            self.defense,
            self.weapon,
            self.weapon,
            self.defense,
            self.weapon,
            self.weapon,
            self.weapon,
            self.hull,
        )


class TechGroup2SparseB(TechGroup2):
    def __init__(self):
        super(TechGroup2SparseB, self).__init__()
        self.enqueue(
            self.armor,
            self.hull,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.defense,
            self.weapon,
            self.weapon,
            self.defense,
            self.defense,
            self.weapon,
            self.weapon,
            self.weapon,
            self.hull,
        )


class TechGroup3(TechGroup):
    def __init__(self):
        super(TechGroup3, self).__init__()
        self.hull.extend([
            "SHP_ASTEROID_REFORM",
            "SHP_HEAVY_AST_HULL",
            "SHP_CONTGRAV_MAINT",
        ])
        self.economy.extend([
            "PRO_INDUSTRY_CENTER_I",
            "GRO_GENETIC_ENG",
            "GRO_GENETIC_MED",
            "GRO_XENO_GENETICS",
            "LRN_QUANT_NET",
            "PRO_SOL_ORB_GEN",
            "PRO_INDUSTRY_CENTER_II",
            "GRO_XENO_HYBRIDS",
            "CON_ORBITAL_HAB",
            "CON_NDIM_STRC",
            "GRO_LIFECYCLE_MAN",
        ])
        self.defense.extend([
            "DEF_DEFENSE_NET_2",
            "DEF_DEFENSE_NET_REGEN_1",
            "DEF_PLAN_BARRIER_SHLD_1",
            "DEF_GARRISON_3",
            "SPY_DETECT_3",
            "DEF_DEFENSE_NET_REGEN_2",
            "DEF_PLAN_BARRIER_SHLD_2",
            "DEF_DEFENSE_NET_3",
            "DEF_SYST_DEF_MINE_1",
            "DEF_PLAN_BARRIER_SHLD_3",
        ])
        self.misc.extend([
            "SHP_BASIC_DAM_CONT",
            "SHP_INTSTEL_LOG",
            "SHP_FLEET_REPAIR",
            "SHP_IMPROVED_ENGINE_COUPLINGS",
            "SHP_REINFORCED_HULL",
            "SHP_DEFLECTOR_SHIELD",
            "CON_CONTGRAV_ARCH",
            "CON_FRC_ENRG_STRC",
            "SHP_N_DIMENSIONAL_ENGINE_MATRIX",
        ])
        self.armor.extend([
            "SHP_DIAMOND_PLATE",
        ])
        self.weapon.extend([
            "SHP_WEAPON_3_1",
            "SHP_WEAPON_3_2",
            "SHP_FIGHTERS_3",
            "SHP_WEAPON_3_3",
            "SHP_WEAPON_3_4",
        ])


class TechGroup3A(TechGroup3):
    def __init__(self):
        super(TechGroup3A, self).__init__()
        self.enqueue(
            self.hull,
            self.economy,
            self.defense,
            self.defense,
            self.economy,
            self.economy,
            self.economy,
            self.defense,
            self.misc,
            self.defense,
            self.economy,
            self.misc,
            self.misc,
            self.economy,
            self.misc,
            self.hull,
            self.hull,
            self.misc,
            self.economy,
            self.hull,
            self.economy,
            self.defense,
            self.misc,
            self.armor,
            self.defense,
            self.defense,
            self.defense,
            self.misc,
            self.economy,
            self.misc,
            self.defense,
            self.defense,
            self.economy,
            self.misc,
            self.economy,
            self.weapon,
            self.weapon,
            self.weapon,
            self.weapon,
            self.weapon
        )


class TechGroup3B(TechGroup3):
    def __init__(self):
        super(TechGroup3B, self).__init__()
        self.enqueue(
            self.hull,
            self.economy,
            self.defense,
            self.defense,
            self.economy,
            self.economy,
            self.economy,
            self.weapon,
            self.weapon,
            self.defense,
            self.misc,
            self.defense,
            self.economy,
            self.weapon,
            self.misc,
            self.misc,
            self.economy,
            self.misc,
            self.weapon,
            self.weapon,
            self.hull,
            self.hull,
            self.misc,
            self.economy,
            self.hull,
            self.economy,
            self.defense,
            self.misc,
            self.armor,
            self.defense,
            self.defense,
            self.defense,
            self.misc,
            self.economy,
            self.misc,
            self.defense,
            self.defense,
            self.economy,
            self.misc,
            self.economy
        )


class TechGroup3Sparse(TechGroup3):
    def __init__(self):
        super(TechGroup3Sparse, self).__init__()
        self.enqueue(
            self.hull,
            self.defense,
            self.defense,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.economy,
            self.misc,
            self.economy,
            self.weapon,
            self.weapon,
            self.defense,
            self.defense,
            self.weapon,
            self.misc,
            self.misc,
            self.economy,
            self.misc,
            self.weapon,
            self.weapon,
            self.hull,
            self.hull,
            self.misc,
            self.economy,
            self.hull,
            self.economy,
            self.defense,
            self.misc,
            self.armor,
            self.defense,
            self.defense,
            self.defense,
            self.misc,
            self.economy,
            self.misc,
            self.defense,
            self.defense,
            self.economy,
            self.misc,
        )


class TechGroup4(TechGroup):
    def __init__(self):
        super(TechGroup4, self).__init__()
        self.hull.extend([
            "SHP_FRC_ENRG_COMP",
            "SHP_MASSPROP_SPEC",
            "SHP_SCAT_AST_HULL",
        ])
        self.enqueue(
            self.hull,
            self.hull
        )


class TechGroup5(TechGroup):
    def __init__(self):
        super(TechGroup5, self).__init__()
        self._tech_queue.extend([
            "DEF_GARRISON_4",
            "DEF_PLAN_BARRIER_SHLD_4",
            "LRN_XENOARCH",
            "LRN_GRAVITONICS",
            "LRN_STELLAR_TOMOGRAPHY",
            "LRN_ENCLAVE_VOID",
            "SHP_MONOMOLEC_LATTICE",
            "SHP_ADV_DAM_CONT",
            "LRN_TIME_MECH",
            "SPY_DETECT_4",
            "SHP_CONT_SYMB",
            "SHP_MONOCELL_EXP",
            "GRO_CYBORG",
            "GRO_TERRAFORM",
            "GRO_ENERGY_META",
            "LRN_DISTRIB_THOUGHT",
            "SHP_WEAPON_4_1",
            "SHP_WEAPON_4_2",
            "SHP_FIGHTERS_4",
            "SHP_WEAPON_4_3",
            "PRO_INDUSTRY_CENTER_III",
            "PRO_SINGULAR_GEN",
            "SHP_QUANT_ENRG_MAG",
            "SHP_PLASMA_SHIELD",
            "SHP_ENRG_BOUND_MAN",
            "SHP_WEAPON_4_4",
            "PRO_NEUTRONIUM_EXTRACTION",
            "SHP_SOLAR_CONT",
            "CON_CONC_CAMP",
            "LRN_ART_BLACK_HOLE",
            "DEF_SYST_DEF_MINE_2",
            "DEF_SYST_DEF_MINE_3",
            "LRN_PSY_DOM",
            "SHP_BLACKSHIELD",
            "DEF_PLAN_BARRIER_SHLD_5",
            "SPY_DETECT_5",
            "GRO_GAIA_TRANS",
            "CON_ART_PLANET",
        ])


def test_tech_integrity():
    """Check the TechGroups for integrity.

    Try to get all tech lists by querying all the TechGroups.
    Any error is displayed in chat window.
    Also checks if all techs exist and displays error if invalid tech name encountered.
    """
    tech_groups = [
        TechGroup,
        TechGroup1,
        TechGroup1a,
        TechGroup1b,
        TechGroup1SparseA,
        TechGroup1SparseB,
        TechGroup1SparseC,
        TechGroup2,
        TechGroup2A,
        TechGroup2B,
        TechGroup2SparseA,
        TechGroup2SparseB,
        TechGroup3,
        TechGroup3A,
        TechGroup3B,
        TechGroup3Sparse,
        TechGroup4,
        TechGroup5
    ]
    debug("Checking TechGroup integrity...")
    for group in tech_groups:
        debug("Checking %s: " % group.__name__)
        error_occured = False
        this_group = group()
        techs = this_group.get_techs()
        for tech in techs:
            if not fo.getTech(tech):
                warn("In %s: Tech %s seems not to exist!" % (group.__name__, tech))
                error_occured = True
        for err in this_group.get_errors():
            warn(err, exc_info=True)
            error_occured = True
        if not error_occured:
            debug("Seems to be OK!")


def sparse_galaxy_techs(index):
    # return primary_meta_techs()
    result = []
    debug("Choosing Research Techlist Index %d" % index)
    if index == 0:
        result = TechGroup1a().get_techs()  # early org_hull
        result += TechGroup2A().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3A().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 1:
        result = TechGroup1b().get_techs()  # early _lrn_artif_minds
        result += TechGroup2B().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3B().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 2:
        result = TechGroup1SparseA().get_techs()  # early _lrn_artif_minds
        result += TechGroup2SparseA().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3Sparse().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 3:
        result = TechGroup1SparseB().get_techs()  # early org_hull
        result += TechGroup2SparseB().get_techs()
        result += TechGroup3A().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 4:
        result = TechGroup1SparseC().get_techs()  # early pro_sent_auto
        result += TechGroup2SparseB().get_techs()
        result += TechGroup3B().get_techs()  # faster plasma weaps
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    return result


def primary_meta_techs(index=0):
    """
    Primary techs for all categories.
    """
    # index = 1 - index
    result = []

    debug("Choosing Research Techlist Index %d" % index)
    if index == 0:
        result = TechGroup1a().get_techs()  # early org_hull
        result += TechGroup2A().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3A().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 1:
        result = TechGroup1b().get_techs()  # early org_hull
        result += TechGroup2B().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3B().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 2:
        result = TechGroup1a().get_techs()  # early org_hull
        result += TechGroup2B().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3A().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 3:
        result = TechGroup1b().get_techs()  # early org_hull
        result += TechGroup2A().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3B().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    elif index == 4:
        result = TechGroup1a().get_techs()  # early org_hull
        result += TechGroup2A().get_techs()  # prioritizes growth & defense over weapons
        result += TechGroup3B().get_techs()
        result += TechGroup4().get_techs()
        result += TechGroup5().get_techs()  #
    return result

# the following is just for reference
# "CON_ARCH_MONOFILS",
# "CON_ARCH_PSYCH",
# "CON_ART_HEAVENLY",
# "CON_ART_PLANET",
# "CON_ASYMP_MATS",
# "CON_CONC_CAMP",
# "CON_CONTGRAV_ARCH",
# "CON_FRC_ENRG_CAMO",
# "CON_FRC_ENRG_STRC",
# "CON_GAL_INFRA",
# "CON_NDIM_STRC",
# "CON_ORBITAL_CON",
# "CON_ORBITAL_HAB",
# "CON_ORGANIC_STRC",
# "CON_PLANET_DRIVE",
# "CON_STARGATE",
# "CON_TRANS_ARCH",
# "DEF_DEFENSE_NET_1",
# "DEF_DEFENSE_NET_2",
# "DEF_DEFENSE_NET_3",
# "DEF_DEFENSE_NET_REGEN_1",
# "DEF_DEFENSE_NET_REGEN_2",
# "DEF_GARRISON_1",
# "DEF_GARRISON_2",
# "DEF_GARRISON_3",
# "DEF_GARRISON_4",
# "DEF_PLANET_CLOAK",
# "DEF_PLAN_BARRIER_SHLD_1",
# "DEF_PLAN_BARRIER_SHLD_2",
# "DEF_PLAN_BARRIER_SHLD_3",
# "DEF_PLAN_BARRIER_SHLD_4",
# "DEF_PLAN_BARRIER_SHLD_5",
# "DEF_ROOT_DEFENSE",
# "DEF_SYST_DEF_MINE_1",
# "DEF_SYST_DEF_MINE_2",
# "DEF_SYST_DEF_MINE_3",
# "GRO_ADV_ECOMAN",
# "GRO_BIOTERROR",
# "GRO_CYBORG",
# "GRO_ENERGY_META",
# "GRO_GAIA_TRANS",
# "GRO_GENETIC_ENG",
# "GRO_GENETIC_MED",
# "GRO_LIFECYCLE_MAN",
# "GRO_NANOTECH_MED",
# "GRO_NANO_CYBERNET",
# "GRO_PLANET_ECOL",
# "GRO_SUBTER_HAB",
# "GRO_SYMBIOTIC_BIO",
# "GRO_TERRAFORM",
# "GRO_TRANSORG_SENT",
# "GRO_XENO_GENETICS",
# "GRO_XENO_HYBRIDS",
# "LRN_ALGO_ELEGANCE",
# "LRN_ARTIF_MINDS",
# "LRN_ART_BLACK_HOLE",
# "LRN_DISTRIB_THOUGHT",
# "LRN_ENCLAVE_VOID",
# "LRN_EVERYTHING",
# "LRN_FORCE_FIELD",
# "LRN_GATEWAY_VOID",
# "LRN_GRAVITONICS",
# "LRN_MIND_VOID",
# "LRN_NDIM_SUBSPACE",
# "LRN_OBSERVATORY_I",
# "LRN_PHYS_BRAIN",
# "LRN_PSIONICS",
# "LRN_PSY_DOM",
# "LRN_QUANT_NET",
# "LRN_SPATIAL_DISTORT_GEN",
# "LRN_STELLAR_TOMOGRAPHY",
# "LRN_TIME_MECH",
# "LRN_TRANSCEND",
# "LRN_TRANSLING_THT",
# "LRN_UNIF_CONC",
# "LRN_XENOARCH",
# "PRO_EXOBOTS",
# "PRO_FUSION_GEN",
# "PRO_INDUSTRY_CENTER_I",
# "PRO_INDUSTRY_CENTER_II",
# "PRO_INDUSTRY_CENTER_III",
# "PRO_MICROGRAV_MAN",
# "PRO_NANOTECH_PROD",
# "PRO_NDIM_ASSMB",
# "PRO_NEUTRONIUM_EXTRACTION",
# "PRO_ORBITAL_GEN",
# "PRO_ROBOTIC_PROD",
# "PRO_SENTIENT_AUTOMATION",
# "PRO_SINGULAR_GEN",
# "PRO_SOL_ORB_GEN",
# "PRO_ZERO_GEN",
# "SHP_ADV_DAM_CONT",
# "SHP_ANTIMATTER_TANK",
# "SHP_ASTEROID_HULLS",
# "SHP_ASTEROID_REFORM",
# "SHP_BASIC_DAM_CONT",
# "SHP_BIOADAPTIVE_SPEC",
# "SHP_BIOTERM",
# "SHP_BLACKSHIELD",
# "SHP_CAMO_AST_HULL",
# "SHP_CONTGRAV_MAINT",
# "SHP_CONT_BIOADAPT",
# "SHP_CONT_SYMB",
# "SHP_DEATH_SPORE",
# "SHP_DEFLECTOR_SHIELD",
# "SHP_DEUTERIUM_TANK",
# "SHP_DIAMOND_PLATE",
# "SHP_DOMESTIC_MONSTER",
# "SHP_ENDOCRINE_SYSTEMS",
# "SHP_ENDOSYMB_HULL",
# "SHP_ENRG_BOUND_MAN",
# "SHP_FLEET_REPAIR",
# "SHP_FRC_ENRG_COMP",
# "SHP_GAL_EXPLO",
# "SHP_HEAVY_AST_HULL",
# "SHP_IMPROVED_ENGINE_COUPLINGS",
# "SHP_INTSTEL_LOG",
# "SHP_MASSPROP_SPEC",
# "SHP_MIDCOMB_LOG",
# "SHP_MIL_ROBO_CONT",
# "SHP_MINIAST_SWARM",
# "SHP_MONOCELL_EXP",
# "SHP_MONOMOLEC_LATTICE",
# "SHP_MULTICELL_CAST",
# "SHP_MULTISPEC_SHIELD",
# "SHP_NANOROBO_MAINT",
# "SHP_NOVA_BOMB",
# "SHP_N_DIMENSIONAL_ENGINE_MATRIX",
# "SHP_ORG_HULL",
# "SHP_PLASMA_SHIELD",
# "SHP_QUANT_ENRG_MAG",
# "SHP_REINFORCED_HULL",
# "SHP_ROOT_AGGRESSION",
# "SHP_ROOT_ARMOR",
# "SHP_SCAT_AST_HULL",
# "SHP_SENT_HULL",
# "SHP_SINGULARITY_ENGINE_CORE",
# "SHP_SOLAR_CONT",
# "SHP_SPACE_FLUX_DRIVE",
# "SHP_TRANSSPACE_DRIVE",
# "SHP_WEAPON_1_2",
# "SHP_WEAPON_1_3",
# "SHP_WEAPON_1_4",
# "SHP_WEAPON_2_1",
# "SHP_WEAPON_2_2",
# "SHP_WEAPON_2_3",
# "SHP_WEAPON_2_4",
# "SHP_WEAPON_3_1",
# "SHP_WEAPON_3_2",
# "SHP_WEAPON_3_3",
# "SHP_WEAPON_3_4",
# "SHP_WEAPON_4_1",
# "SHP_WEAPON_4_2",
# "SHP_WEAPON_4_3",
# "SHP_WEAPON_4_4",
# "SHP_XENTRONIUM_PLATE",
# "SHP_ZORTRIUM_PLATE",
# "SPY_DETECT_1",
# "SPY_DETECT_2",
# "SPY_DETECT_3",
# "SPY_DETECT_4",
# "SPY_DETECT_5",
# "SPY_DIST_MOD",
# "SPY_LIGHTHOUSE",
# "SPY_PLANET_STEALTH_MOD",
# "SPY_ROOT_DECEPTION",
# "SPY_STEALTH_1",
# "SPY_STEALTH_2",
# "SPY_STEALTH_3",
# "SPY_STEALTH_4",
