## This file is included from client/* and server/* Makefiles

# Remake convenience libs if needed

%-common.a: force
	make -C $(@D)

force : ;

