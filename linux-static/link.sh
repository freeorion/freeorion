#!/bin/sh 
#GV=/data1/archiv/games/freeorion/graphviz-2.12
#GV=/data1/archiv/games/freeorion/graphviz-2.16.1
GV=../graphviz-2.18
GV_LIBS=$GV/lib/graph/.libs

#BOOST_SUFFIX=-gcc42-mt-1_35
#BOOST_SUFFIX=-gcc42-1_34_1
BOOST_SUFFIX=-gcc43-mt
#BOOST_SUFFIX=-gcc42-mt

#CXX=g++-4.2
#CC=gcc-4.2
CXX=g++-4.3
CC=gcc-4.3


if [ $# -ge 1 ]; then
    FO=0
    FOD=0
    FOCA=0    
    FOPYCA=0

    while [ $# -ge 1 ]; do
        case $1 in
            freeorionca-pythonstatic) FOPYCA=1;;
            freeorionca-static) FOCA=1;;
            freeoriond-static)  FOD=1;;
            freeorion-static)   FO=1;;
        esac

        shift
    done
else
    FO=1
    FOD=1
    FOCA=1
    FOPYCA=0
fi


if [ $FO == 1 ] && [ 1 == 1 ]; then
echo Linking freeorion-static

#UI/FocusSelector-human.o 
$CC -I ${GV}/lib/gvc/ -c -o static-graphviz.o static-graphviz.c

OBJS="combat/Combat.o combat/CombatOrder.o Empire/Empire.o Empire/EmpireManager.o Empire/ResourcePool.o network/Message.o network/MessageQueue.o network/Networking.o UI/StringTable.o UI/ShaderProgram-human.o universe/Building.o universe/Condition.o universe/ConditionParser1.o universe/ConditionParser2.o universe/ConditionParser.o universe/Effect.o universe/EffectParser.o universe/Enums.o universe/Fleet.o universe/Meter.o universe/ParserUtil.o universe/Planet.o universe/PopCenter.o universe/Predicates.o universe/ResourceCenter.o universe/Ship.o universe/ShipDesign.o universe/Special.o universe/System.o universe/Tech.o universe/TopLevelParsers.o universe/UniverseObject.o universe/ValueRef.o universe/ValueRefParser.o util/DataTable.o util/GZStream.o util/MultiplayerCommon.o util/OptionsDB.o util/Order.o util/OrderSet.o util/Process.o util/Random.o util/Serialize.o util/SitRepEntry.o util/VarText.o util/Version.o util/binreloc.o util/Directories.o util/XMLDoc.o log4cpp/src/Appender.o log4cpp/src/AppenderSkeleton.o log4cpp/src/BasicLayout.o log4cpp/src/Category.o log4cpp/src/CategoryStream.o log4cpp/src/Configurator.o log4cpp/src/FactoryParams.o log4cpp/src/FileAppender.o log4cpp/src/HierarchyMaintainer.o log4cpp/src/LayoutAppender.o log4cpp/src/Localtime.o log4cpp/src/LoggingEvent.o log4cpp/src/NDC.o log4cpp/src/PatternLayout.o log4cpp/src/Priority.o log4cpp/src/StringUtil.o log4cpp/src/TimeStamp.o log4cpp/src/PThreads.o client/ClientApp-human.o client/ClientFSMEvents-human.o client/human/HumanClientFSM-human.o client/human/HumanClientApp-human.o client/human/chmain-human.o network/ClientNetworking-human.o UI/About-human.o UI/BuildDesignatorWnd-human.o UI/ChatWnd-human.o UI/ClientUI-human.o UI/CUIControls-human.o UI/CUIDrawUtil-human.o UI/CUIStyle-human.o UI/CUIWnd-human.o UI/FleetButton-human.o UI/FleetWnd-human.o UI/GalaxySetupWnd-human.o UI/InGameMenu-human.o UI/InfoPanels-human.o UI/IntroScreen-human.o UI/LinkText-human.o UI/CollisionMeshConverter-human.o UI/CombatWnd-human.o UI/MapWnd-human.o UI/MultiplayerLobbyWnd-human.o UI/OptionsWnd-human.o UI/DesignWnd-human.o UI/ProductionWnd-human.o UI/QueueListBox-human.o UI/ResearchWnd-human.o UI/ServerConnectWnd-human.o UI/SidePanel-human.o UI/SitRepPanel-human.o UI/Sound-human.o UI/SystemIcon-human.o UI/TechTreeWnd-human.o UI/TurnProgressWnd-human.o universe/Universe-human.o util/AppInterface-human.o UI/EncyclopediaDetailPanel-human.o UI/PagedGeometry/*.o combat/OpenSteer/*.o "

#$GV/plugin/pango/.libs/libgvplugin_pango_C.a \
#$GV/lib/gvc/.libs/demand_loading.o \

#
#-Wl,--as-needed \

if ! \
$CXX \
-o freeorion-static \
-Wl,--start-group \
$OBJS \
-static-libgcc \
static-graphviz.o \
$GV/plugin/dot_layout/.libs/libgvplugin_dot_layout.a \
$GV/plugin/neato_layout/.libs/libgvplugin_neato_layout_C.a \
$GV/lib/gvc/.libs/libgvc_C.a \
$GV/lib/cdt/.libs/libcdt_C.a \
$GV/lib/graph/.libs/libgraph_C.a \
$GV/lib/pathplan/.libs/libpathplan_C.a \
-LGG -L/usr/lib -L/usr/local/lib \
-L../bullet-2.73/src \
-Wl,-static \
-pthread \
-lbulletdynamics -lbulletcollision -lbulletmath \
-ldirectfb -ldirect -lfusion -lpulse -lpulse-simple \
-lrt -lasyncns -lresolv \
-lasound -lopenal -lfreetype -lz -lalut -lopenal -lvorbisfile -lvorbis \
-logg -lnsl -lvga -lx86 \
-lartsc -laudio -lcaca -lcucul -lmng -llcms \
-lboost_system${BOOST_SUFFIX} -lboost_thread${BOOST_SUFFIX} -lboost_signals${BOOST_SUFFIX} -lboost_filesystem${BOOST_SUFFIX} -lboost_serialization${BOOST_SUFFIX} -lboost_iostreams${BOOST_SUFFIX} \
-lesd -lICE -lexpat -ljpeg -lutil -lpng12 \
-lXt -lSM -lXaw7 -lXaw3d -lXmu -lXpm -lzzip -lXext -lX11 \
-lxcb-xlib -lxcb \
-lslang -lXau  \
-laa -lncurses -lgpm \
-lstdc++ -lm  \
-Wl,-dy -lGL -lGLU -lGiGi -lGiGiOgre -lOgreMain -lfreeimage -lOIS \
-Wl,--end-group \
;
then
    exit 1
fi

#-lfontconfig  \

# 
# -lSDL -lasound -lopenal   
fi #[ $FO == 1 ]



if [ $FOD == 1 ] &&[ 1 == 1 ]; then
echo linking freeoriond-static

if ! \
$CXX \
-o freeoriond-static \
-pthread \
-Wl,--as-needed \
-Wl,--start-group  \
-static-libgcc \
combat/Combat.o combat/CombatOrder.o Empire/Empire.o Empire/EmpireManager.o Empire/ResourcePool.o network/Message.o network/MessageQueue.o network/Networking.o UI/StringTable.o universe/Building.o universe/Condition.o universe/ConditionParser1.o universe/ConditionParser2.o universe/ConditionParser.o universe/Effect.o universe/EffectParser.o universe/Enums.o universe/Fleet.o universe/Meter.o universe/ParserUtil.o universe/Planet.o universe/PopCenter.o universe/Predicates.o universe/ResourceCenter.o universe/Ship.o universe/ShipDesign.o universe/Special.o universe/System.o universe/Tech.o universe/TopLevelParsers.o universe/UniverseObject.o universe/ValueRef.o universe/ValueRefParser.o util/DataTable.o util/GZStream.o util/MultiplayerCommon.o util/OptionsDB.o util/Order.o util/OrderSet.o util/Process.o util/Random.o util/Serialize.o util/SitRepEntry.o util/VarText.o util/Version.o util/binreloc.o util/Directories.o util/XMLDoc.o combat/CombatSystem-server.o network/ServerNetworking-server.o server/SaveLoad-server.o server/ServerApp-server.o server/ServerFSM-server.o server/dmain-server.o universe/Universe-server.o util/AppInterface-server.o \
log4cpp/src/*.o combat/OpenSteer/*.o \
$GV/lib/gvc/.libs/libgvc_C.a $GV/lib/graph/.libs/libgraph_C.a  \
-LGG -L/usr/lib \
-Wl,--static \
-lGiGi  \
-lboost_system${BOOST_SUFFIX} -lboost_thread${BOOST_SUFFIX} -lboost_signals${BOOST_SUFFIX} -lboost_filesystem${BOOST_SUFFIX} -lboost_iostreams${BOOST_SUFFIX} -lboost_serialization${BOOST_SUFFIX} \
-lstdc++ -lz -lm \
-Wl,--end-group \
;
then
    exit 1
fi

fi





if [ $FOCA == 1 ] && [ 1 == 1 ]; then
    echo Linking freeorionca-static

if ! \
$CXX -o freeorionca-static -pthread \
-Wl,--as-needed \
-Wl,--start-group \
combat/Combat.o combat/CombatOrder.o  Empire/Empire.o Empire/EmpireManager.o Empire/ResourcePool.o network/Message.o network/MessageQueue.o network/Networking.o UI/StringTable.o universe/Building.o universe/Condition.o universe/ConditionParser1.o universe/ConditionParser2.o universe/ConditionParser.o universe/Effect.o universe/EffectParser.o universe/Enums.o universe/Fleet.o universe/Meter.o universe/ParserUtil.o universe/Planet.o universe/PopCenter.o universe/Predicates.o universe/ResourceCenter.o universe/Ship.o universe/ShipDesign.o universe/Special.o universe/System.o universe/Tech.o universe/TopLevelParsers.o universe/UniverseObject.o universe/ValueRef.o universe/ValueRefParser.o util/DataTable.o util/GZStream.o util/MultiplayerCommon.o util/OptionsDB.o util/Order.o util/OrderSet.o util/Process.o util/Random.o util/Serialize.o util/SitRepEntry.o util/VarText.o util/Version.o util/binreloc.o util/Directories.o util/XMLDoc.o client/ClientApp-ai.o client/ClientFSMEvents-ai.o client/AI/AIClientApp-ai.o client/AI/camain-ai.o network/ClientNetworking-ai.o universe/Universe-ai.o util/AppInterface-ai.o AI/AIInterface-ai.o AI/PythonAI-ai.o python/PythonEnumWrapper-ai.o python/PythonUniverseWrapper-ai.o python/PythonEmpireWrapper-ai.o python/PythonLoggingWrapper-ai.o \
log4cpp/src/*.o  combat/OpenSteer/*.o \
-LGG -L/usr/lib -L/usr/lib/python2.5/config \
-static-libgcc  \
-Wl,-static \
-lGiGi \
-lboost_system${BOOST_SUFFIX} -lboost_thread${BOOST_SUFFIX} -lboost_signals${BOOST_SUFFIX} -lboost_filesystem${BOOST_SUFFIX} -lboost_serialization${BOOST_SUFFIX} -lboost_iostreams${BOOST_SUFFIX} -lboost_python${BOOST_SUFFIX} \
-lstdc++ -lz -lm \
-Wl,--end-group \
-Wl,-dy -lpython2.5 \
;
then
    exit 1
fi
fi


if [ $FOPYCA == 1 ] ; then
echo Linking freeorionca-pythonstatic

if ! \
    $CXX -o freeorionca-pythonstatic -pthread \
    -Wl,--as-needed \
    -Wl,--start-group \
    combat/Combat.o Empire/Empire.o Empire/EmpireManager.o Empire/ResourcePool.o network/Message.o network/MessageQueue.o network/Networking.o network/boost/error_code.o UI/StringTable.o universe/Building.o universe/Condition.o universe/ConditionParser1.o universe/ConditionParser2.o universe/ConditionParser.o universe/Effect.o universe/EffectParser.o universe/Enums.o universe/Fleet.o universe/Meter.o universe/ParserUtil.o universe/Planet.o universe/PopCenter.o universe/Predicates.o universe/ResourceCenter.o universe/Ship.o universe/ShipDesign.o universe/Special.o universe/System.o universe/Tech.o universe/TopLevelParsers.o universe/UniverseObject.o universe/ValueRef.o universe/ValueRefParser.o util/DataTable.o util/GZStream.o util/MultiplayerCommon.o util/OptionsDB.o util/Order.o util/OrderSet.o util/Process.o util/Random.o util/Serialize.o util/SitRepEntry.o util/VarText.o util/Version.o util/binreloc.o util/Directories.o util/XMLDoc.o client/ClientApp-ai.o client/ClientFSMEvents-ai.o client/AI/AIClientApp-ai.o client/AI/camain-ai.o network/ClientNetworking-ai.o universe/Universe-ai.o util/AppInterface-ai.o AI/AIInterface-ai.o AI/PythonAI-ai.o python/PythonEnumWrapper-ai.o python/PythonUniverseWrapper-ai.o python/PythonEmpireWrapper-ai.o python/PythonLoggingWrapper-ai.o \
    log4cpp/src/*.o \
    -LGG -L/usr/lib -L/usr/lib/python2.5/config \
    -static-libgcc  \
    -Wl,-static \
    -lGiGi \
    -lboost_system${BOOST_SUFFIX} -lboost_thread${BOOST_SUFFIX} -lboost_signals${BOOST_SUFFIX} \
    -lboost_filesystem${BOOST_SUFFIX} -lboost_serialization${BOOST_SUFFIX} -lboost_iostreams${BOOST_SUFFIX} \
    -lboost_python-py25 \
    -lstdc++ -lz -lm \
    -lpython2.5 \
-Wl,--end-group \
;
then
    exit 1
fi
fi



exit
./copy_binaries.sh
