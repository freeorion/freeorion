# pylab is part of matplotlib -- http://matplotlib.org/
# for necessary Windows dependencies see http://stackoverflow.com/a/18280786

# windows installation by Cjkjvfnby http://www.freeorion.org/forum/memberlist.php?mode=viewprofile&u=34706
#  I use current version 1.4.2 for python 2.7 *32
#
# If you have compiler installed
#   pip install matplotlib
#
# Otherwise you need to get precompiled binaries:
# download and install matplotlib-1.4.2.win32-py2.7.exe from http://matplotlib.org/downloads.html
# pip install python-dateutil pyparsing pillow  # if pillow does not want to install use numpy instruction.
# download numpy-1.9.1+mkl-cp27-none-win32.whl from  http://www.lfd.uci.edu/~gohlke/pythonlibs/#numpy
# navigate to folder: pip install numpy-1.9.1+mkl-cp27-none-win32.whl
# Change dataDir to '<user>/AppData/Roaming/'

from pylab import *
import os
import sys
from glob import glob
import traceback

dataDir = (os.environ.get('HOME', "") + "/.freeorion" ) if os.name != 'posix' else (os.environ.get('XDG_DATA_HOME', os.environ.get('HOME', "") + "/.local/share") + "/freeorion") 
graphDir = dataDir

fileRoot = "game1"
saveFile = True
turnsP = None
turnsAI = None
rankings = []


def show_only_some(x, pos):
    val = int(x)
    if val in [3, 5, 30, 50, 80, 300, 500, 800, 3000, 5000, 8000]:
        return str(val)
    else:
        return ''

doPlotTypes = ["PP + 2RP"] + ["PP"] + ["RP"] + ["RP_Ratio"]  # +[ "ShipCount"]


def parse_file(file_name, ai=True):
        print "processing file ", file_name
        sys.stdout.flush()
        got_colors = False
        got_species = False
        got_name = False
        data = {"PP": [], "RP": [], "RP_Ratio": [], "ShipCount": [], "turnsP": [], "turnPP": [], "PP + 2RP": []}
        details = {'color': {1, 1, 1, 1}, 'name': "", 'species': ""}
        with open(file_name, 'r') as lf:
            while True:
                line = lf.readline()
                if not line:
                    break
                if not got_colors and "EmpireColors:" in line:
                    colors = line.split("EmpireColors:")[1].split()
                    if len(colors) == 4:
                        got_colors = True
                        if type(colors[0]) == type("0"):
                            details['color'] = tuple(map(lambda x: float(x) / 255.0, colors))
                        else:
                            details['color'] = tuple(map(lambda x: float(ord(x[0])) / 255.0, colors))
                if ai and not got_species and "CapitalID:" in line:
                    got_species = True
                    details['species'] = line.split("Species:")[1].strip()
                if ai and not got_name and "EmpireID:" in line:
                    got_name = True
                    details['name'] = line.split("Name:")[1].split("Turn:")[0].strip()
                if "Current Output (turn" in line:
                    info = line.split("Current Output (turn")[1]
                    parts = info.split(')')
                    data['turnsP'].append((int(parts[0])))
                    data['turnPP'].append((int(parts[0]), float(parts[1].split('/')[-1])))
                    rppp = parts[1].split('(')[-1].split('/')
                    data['PP'].append(float(rppp[1]))
                    data['RP'].append(float(rppp[0]))
                    data['PP + 2RP'].append(float(rppp[1]) + 2 * float(rppp[0]))
                    data['RP_Ratio'].append(float(rppp[0]) / (float(rppp[1]) + 0.001))
                if "Empire Ship Count:" in line:
                    data['ShipCount'].append(int(line.split("Empire Ship Count:")[1]))
        return data, details

allData = {}
species = {}
empires = []
empireColors = {}
playerName = "Player"

logfiles = sorted(glob(dataDir + os.sep + "A*.log"))
A1log = glob(dataDir + os.sep + "AI_1.log")


if not os.path.exists(dataDir + os.sep + "freeorion.log"):
    print "can't find freeorion.log"
elif A1log and (A1log[0] in logfiles) and (os.path.getmtime(dataDir + os.sep + "freeorion.log") < os.path.getmtime(A1log[0]) - 300):
    print "freeorion.log file is stale ( more than 5 minutes older than AI_1.log) and will be skipped"
else:
    data, details = parse_file(dataDir + os.sep + "freeorion.log", False)
    if len(data.get('PP', [])) > 0:
        allData[playerName] = data
        empireColors[playerName] = details['color']

logfiles = sorted(glob(dataDir + os.sep + "A*.log"))
A1log = glob(dataDir + os.sep + "AI_1.log")
if A1log and A1log[0] in logfiles:
    A1Time = os.path.getmtime(A1log[0])
    for path in logfiles[::-1]:
        logtime = os.path.getmtime(path)
        # print "path ", path, "logtime diff: %.1f"%(A1Time -logtime)
        if logtime < A1Time - 300:
            del logfiles[logfiles.index(path)]
            print "skipping stale logfile ", path
for lfile in logfiles:
    try:
        data, details = parse_file(lfile, True)
        allData[details['name']] = data
        empireColors[details['name']] = details['color']
        species[details['name']] = details['species']
    except:
        print "error processing %s" % lfile
        print "Error: exception triggered and caught: ", traceback.format_exc()

print
for plotType in doPlotTypes:
    
    print "--------------------"
    if plotType == "PP":
        caption = "Production"
    elif plotType == "RP":
        caption = "Research"
    elif plotType == "RP_Ratio":
        caption = "Res:Prod Ratio"
    elif plotType == "RP_Ratio":
        caption = "Res:Prod Ratio"
    else:
        caption = plotType
    figure(figsize=(10, 6))
    ax = gca()
    ymin = 9999
    ymax = 0
    rankings = []
    turns = []
    
    pdata = allData.get(playerName, {}).get(plotType, [])
    if pdata:
        ymin = min(ymin, min(pdata))
        ymax = max(ymax, max(pdata))
        turns = allData.get(playerName, {}).get('turnsP', [])
        
    for empireName, data in allData.items():
        if empireName == playerName:
            continue
        adata = data.get(plotType, [])
        if adata:
            rankings.append((adata[-1], empireName))
            thisMin = min(adata)
            if thisMin > 0:
                ymin = min(ymin, thisMin)
            ymax = max(ymax, max(adata))
            if not turns:
                turns = data.get('turnsP', [])

    if not turns:
        if len(allData) == 0:
            break
        turns = range(1, len(allData.values()[0].get(plotType, [])) + 1)
    rankings.sort()
    xlabel('Turn')
    ylabel(plotType)
    title(caption + ' Progression')
    if ymax >= 100:
        ax.set_yscale('log', basey=10)
      
    if playerName not in allData:
        print "\t\t\tcan't find playerData in allData\n"
    else:
        if playerName in empireColors:
            turnsP = allData[playerName].get("turnsP", [])
            thisData = allData.get(playerName, {}).get(plotType, [])
            print "plotting with color for player: ", playerName, "data min/max: ", min(allData[playerName].get(plotType, [])), ' | ', max(allData[playerName].get(plotType, []))
            plot(turnsP, thisData, 'o-', color=empireColors[playerName], label="%s - %.1f" % (playerName, sum(thisData)), linewidth=2.0)
        else:
            print "plotting withOUT color for player: ", playerName, "data min/max: ", min(allData[playerName].get(plotType, [])), ' | ', max(allData[playerName].get(plotType, []))
            plot(turnsP, allData[playerName].get(plotType, []), 'bx-', label=playerName, linewidth=2.0)
    print "Ranked by ", plotType
    for rank, name in rankings[::-1]:
        print name
        if name in empireColors:
            adata = allData[name].get(plotType, [])
            plot(range(turns[0], turns[0] + len(adata)), adata, color=empireColors[name], label="%s: %s - %.1f" % (name, species[name], sum(adata)), linewidth=2.0)
        else:
            print "can't find empire color for ", name
            # plot(range(turns[0], turns[0]+len(allData[name])), allData[name].get(plotType, []), label="(%d) "%(empires.index(name)+1)+name+" : "+species[name], linewidth=2.0)
    # legend(loc='upper left',prop={"size":'medium'})
    legend(loc='upper left', prop={"size": 9}, labelspacing=0.2)
    xlabel('Turn')
    ylabel(plotType)
    title(caption + ' Progression ')
    x1, x2, y1, y2 = axis()
    newY2 = y2
    if plotType in ["PP + 2RP", "PP", "RP", "ShipCount"]:
        for yi in range(1, 10):
            if 1.05 * ymax < yi * y2 / 10:
                newY2 = yi * y2 / 10
                break
        print "y1: %.1f ; ymin: %.1f ; newY2/100: %.1f" % (y1, ymin, newY2 / 100)
        y1 = max(y1, 4, ymin, newY2 / 100)
    axis((x1, x2, y1, newY2))
    grid(b=True, which='major', color='0.25', linestyle='-')
    grid(b=True, which='minor', color='0.1', linestyle='--')
    ax.yaxis.set_minor_formatter(FuncFormatter(show_only_some))
    ax.yaxis.set_ticks_position('right')
    ax.yaxis.set_ticks_position('both')
    ax.tick_params(labelleft='on')  # for matplotlib versions where 'both' doesn't work
    # ax.tick_params(labelright='on')
    if saveFile:
        savefig(graphDir + os.sep + plotType + "_" + fileRoot + ".png")
    show()
