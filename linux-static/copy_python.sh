#!/bin/sh
cd `dirname $0`
. mdist.config.sh

# Change back into the FreeOrion root directory
cd ..

PY_ROOT=$TARGET_ROOT/application/python2.5
mkdir -p ${PY_ROOT}/lib-dynload/

(
    cp -a /usr/lib/libpython2.5*  ${PY_ROOT}
    cp -a /lib/libutil.so.1 /lib/libutil-2.7.so ${PY_ROOT}

    cd /usr/lib/python2.5/

    PY_MODS="pickle site os warnings posixpath stat UserDict copy_reg types struct linecache codecs re sre_compile sre_constants sre_parse binascii StringIO copy"
    for M in $PY_MODS ; do
        cp -a \
            $M.py* \
            ${PY_ROOT}
        
        DYN="lib-dynload/_$M.so lib-dynload/$M.so"
        for D in $DYN; do
            if [ -e $D ]; then
                cp -a \
                    $D \
                    ${PY_ROOT}
            fi
        done
    done

    
    PY_ENCODINGS="encodings"
    for M in $PY_ENCODINGS ; do
        cp -a \
            $M \
            ${PY_ROOT}
    done
   
)