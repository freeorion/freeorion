# include for mdist.sh
TARGET_ROOT=/tmp/freeorion
TARGET_BIN=${TARGET_ROOT}/application
TARGET_DBG=/tmp/freeorion-debug/application
TARGET_SETUP=${TARGET_ROOT}/setup.data
TARGET_LIB=${TARGET_BIN}/lib

if [ ! -d $TARGET_BIN ]; then
    mkdir -pv $TARGET_BIN
fi

if [ ! -d $TARGET_SETUP ]; then
    mkdir -pv $TARGET_SETUP
fi

if [ ! -d $TARGET_LIB ]; then
    mkdir -pv $TARGET_LIB
fi

if [ ! -d $TARGET_DBG ]; then
    mkdir -pv $TARGET_DBG
fi
