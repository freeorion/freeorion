SCCS-info %W% %E% 

$Id$
     
			The VCG Tool 
			============
	   A Visualization Tool for compiler graphs


DESCRIPTION
     The VCG tool reads a textual and readable specification of a 
     graph and visualizes  the graph.   If  not  all positions of 
     nodes are fixed,  the tool  layouts the graph using  several 
     heuristics as reducing  the number of crossings,  minimizing 
     the  size of  edges,  centering of nodes.  The specification 
     language of  the  VCG  tool is nearly compatible to GRL, the 
     language of the edge tool, but contains many extensions. The 
     VCG tool  allows folding of dynamically or statically speci-
     fied  regions  of the  graph.  It uses  colors and  runs  on 
     X11. (An older version runs on Sunview).

     The VCG tool has been developed and tested on a Sparc ELC with
     SunOs 4.1.3, X11 Release 5 and Release 6, and different ANSI C 
     and K&R C compilers. It has further been tested on Solaris
     (SunOs 5.3, gcc only), on a Silicon Graphics (IRIX 4.0.5), 
     on a IBM R6000 (AIX 2 with AIX Windows), on a HP-UX (X11R5, c89), 
     on a DecStation (ULTRIX, X11R5, gcc only), on Linux (X11R5, gcc),
     and on OSF (X11R5, gcc). A user ported the tool to VAX/VMS,
     but this port is currently not fully integrated.
     For the tests on these machines, see README.SYS. This may also
     help for a setup on other machines.


LICENSE CONDITIONS
     Copyright (C) 1993--1995 by Iris Lemke, Georg Sander, and
                                 the Compare Consortium 

     This work is supported by the ESPRIT project 5399 Compare.
     We thank the Compare Consortium for the permission to distribute
     this software and documentation freely.  You can redistribute 
     it under the terms of the  GNU General Public License as published by
     the  Free Software Foundation;  version 2  of the License.

     The members of the Compare Consortium are ACE Associated Computer 
     Experts bv, GMD Forschungsstelle an der Universitaet Karlsruhe,
     Harlequin Limited, INRIA, STERIA, Stichting Mathematisch Centrum (CWI), 
     and Universitaet des Saarlandes.

     The Compare Consortium will neither assume responsibility for any 
     damages caused by the use of its products, nor accept warranty or 
     update claims. This product is distributed in the hope that it will 
     be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
     of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
     GNU General Public License for more details.  See the file COPYING.

     The software is available per anonymous ftp at ftp.cs.uni-sb.de.
     Contact  sander@cs.uni-sb.de  for additional information.


A SPECIAL REMARK ABOUT LICENSE CONDITIONS
     We would certainly like to continue to distribute the documented 
     sources freely, but currently we have the situation that we cannot
     continue in this programmer-friendly way as before.

     Thus, we have uglified some of the files in the distribution: these 
     are the graph layout modules. These files are not anymore readable 
     for human being, but they are readeable for the compiler. Thus you 
     can compile the sources as before, but you cannot find out anymore 
     how the details of the layout algorithms work.

     This is a compromise. I think this is a better solution than to 
     distribute binaries, because the users still can adapt the tool to
     their computer system. (In the layout modules, normally no adaption
     is necessary). Further, we did not spent too much time with 
     uglification, thus the result should not be too ugly ;-)


MAILING_LIST
     There is a mailing list vcg-users@cs.uni-sb.de that distributes
     mail to all users of the VCG tool.  If you want to be added to 
     this list, please send a request to me (sander@cs.uni-sb.de).
     Then, you will be informed about bugs and new versions of 
     the tool.


FILES
	README       - this file you are reading actually
	README.SYS   - Hints for different systems, how to configure
		       and setup.
	COPYING      - The license conditions for this software.
		       PLEASE READ THIS.
	Makefile     - the Top Level Makefile.
	tMakefile.tpl- template to generate an executive tMakefile  
		       by config.
	config	     - shell script to configure and some
		       additional features
	preconf      - a directory consisting of a lot of preconfiguration
		       files for different architectures.
	demotrue     - a demonstration of a true command, needed
		       by config.
	src	     - directory of the sources of the VCG tool
	man	     - directory of the manual pages 
	doc	     - directory of the documentation files
	demo	     - directory of the sources of some small
		       utilities and demonstrations
	expl	     - directory of example specification for
		       the VCG tool

	

INSTALLATION NOTES

     0) For the installation on a VAX/VMS, see the special remarks
        in preconf/X11VMS/README.vms.

     1) For UNIX systems, there are three different methods of installation.

	   a) Installation by using one of the preconfigurations.
	      This is the fastest and easiest possibility to install.
	      This works only on machines that I know, and may 
	      fail in general.
	      The preconfigured files are in the directory preconf.

	      The SunView-version is installed in /usr/local/bin
	      and /usr/local/man/manl. It is called vcg.

	      The X11-version asks xmkmf (or imake) for the pathes
	      of BINDIR and MANDIR and installs there. You should
	      have a working xmkmf.
	      The X11-version normally uses a courier-14 font as 
	      default font (Exception: ULTRIX: fixed-bold 13). 
	      The X11-version is called xvcg.

           b) Installation by using the config script. This allows
	      to create a very specific setup. The config script
	      asks interactively for different pathes, compilers,
	      options, the default font, etc. Here, you can setup
	      very individually.
	      This may be appropriate, if none of the standard 
	      configurations is appropriate.

           c) Installation by hand. This is only needed, if b) fails
	      for some reason.
	      Installation by hand means that you edit the CHANGE AREA
	      of the files tMakefile, src/globals.h and demo/demo.csh.
	      To do so, you must first create these files, either by
	      using a preconfiguration as mentioned in a), 
	      or by using the config script in b), 
	      or by copying the files
		preconf/tMakefile -> tMakefile
		preconf/globals.h -> src/globals.h
		preconf/demo.csh  -> demo/demo.csh

     2) INSTALLATION BY USING A PRECONFIGURATION

	If you are using gcc to create a standard X11 version of
	VCG that is called xvcg and is installed in the directories
	bin and man/manl in the home of X11, simply type 

		make
		make install 
		make distclean
	Ready.

	Otherwise:

	First make sure that tMakefile does not exist on the top level.
	If it exist, then it remained from a previous try of installation.
	Remove it:

		rm -f tMakefile

	You should decide whether you want to have a X11 version or
	a SunView version, and which compiler you want to use.
	(Currently, the SunView version is not anymore supported.)
	Depending on that, you do one of

		make xvcg_gcc        (for a X11 version made by gcc)
	or	make xvcg_g++        (for a X11 version made by g++ on SunOs)
	or	make xvcg_cc         (for a X11 version made by cc)
	or	make xvcg_c89        (for a X11 version made by c89 on HP-UX)
	or	make vcg_gcc         (for a SunView version, gcc)
	or	make vcg_cc          (for a SunView version, cc)

	On SunOS, gcc should be preferred before g++. But g++ works, too:
        The VCG tool is C++ compatible.

        Note that the SunOS preconfiguration is for SunOS 4.xxx. It
        will also work for Solaris (SunOS 5.3), except that make install
        will probably not work.  See section PROBLEMS. 

	On ULTRIX, there is only a preconfiguration for gcc.

	On IBM AIX systems, where xmkmf was not available, I succeeded
	with
		make xvcg_gcc_noxmkmf
	or
		make xvcg_cc_noxmkmf

	On HP-UX systems, where xmkmf was not available, I succeeded with

		make xvcg_c89_noxmkmf

	The xvcg_* specifies a X11 version, the vcg_*  specifies a 
	SunView version. gcc specifies the GNU ANSI C compiler,
	cc specifies the usual compiler. On HP-UX systems, c89 can
	also be specified (but should be avoided, if possible). 

	The meaning is:
	On SunOS:      cc  =   K&R C 
		       g++ =   the Gnu g++  
	On IRIX:       cc  =   MIPS ucode CC (ANSI C)
	On AIX:        cc  =   XL R6000 CC (ANSI C)
	On HP-UX:      cc  =   K&R C
		       c89 =   POSIX ANSI C 
	On Linux:      cc  =   gcc 
	On OSF1:       cc  =   /usr/bin/cc (near ANSI C) ???

	On success, you should now have a file tMakefile.
	Now continue with step 5)
		
     3) INSTALLATION BY USING config

	Enter /bin/sh config   or  make configure.

	On a ULTRIX machine, enter    sh5 config

	This starts the shell script config, which asks you a lot
	of questions. The questions are self-explained. 
	Don't worry about complaints of missing *.dvi-files,
	*.ps-files, or missing generators like bison, flex, latex, etc.
	In the most cases, these warnings are not relevant.

	After a successful run of config, you should now have a file 
	tMakefile.
	Now continue with step 5)

     4) INSTALLATION BY HAND 

        This is only necessary, if 'make configure' does not work, or
        if you want to make small changes after `make configure'.
	First look for the files tMakefile, src/globals.h and demo/demo.csh.
	If they don't exist, copy the corresponding files from preconf.
		preconf/tMakefile -> tMakefile
		preconf/globals.h -> src/globals.h
		preconf/demo.csh  -> demo/demo.csh

	In these files, there are marked CHANGE AREAS. Please edit these
	according to your needs.

	demo.csh is only used, if you want to see the demonstration 
	(see make test).

     5) On a ULTRIX machine (DecStation), we had some problems with
	paths. On these machines, you should now go into the directory
	src and run the script ultrixpreconf, i.e. enter "csh ultrixpreconf".
	Further,  perhaps the font fixed-bold or the font Courier is not 
	available.  Please edit the file src/globals.h at the place 
        VCG_DEFAULT_FONT to select another font. 
 
        Now you are ready to compile and install. This is done by

		      ( make depend )      
			make xvcg
		      (	make install )
			make test  
		      (	make clean     )
		      (	make distclean )

	Messages like  "sh: parsegen: not found"  can be ignored.

	If the files grammar.y, grammar.l or grammar.h are not found,
	then there is the same path-problem we had on a ULTRIX system.
	Please go to the directory src and run the script "ultrixpreconf".
	Then try again.

	Note: If you use Linux and gnu-make, make install may yield
	a infinite recursion, even if it works correctly. It simply
	does not terminate.
	In Linux, before "make test", you have to adapt the file
	demo/demo.csh by hand, i.e. to change all /bin/echo into
	/usr/bin/echo. If you use a preconfiguration, this is already
	done.

	Further, people reported problems with some versions of gnu-make.
	Although I tried to solve these problems in this new version
	of the VCG tool, it is probably better to use /bin/make instead.
	Setting
		alias make /bin/make

	might help in such situations.

	IMPORTANT: be careful if the environment variables LIBPATH or
	BINPATH are set by your shell. Then, something may not work.
	In this case, unsetenv LIBPATH and unsetenv BINPATH in your
	shell. Further, it may be necessary to set SHELL=/bin/sh
	(or SHELL=sh5 on ULTRIX).

	make depend is normally not necessary. make install does the
	installation. It is a good idea to check that the directories
	exist where you want to install something before you do
	make install.  Not in all cases, the installation directories can 
	be created automatically.  make clean  or  make distclean 
	clean the directories.
	If this was successful, we are ready. If not, see the following,
	what happens in these steps.
	
     6)	If you think it is necessary, type the command at top level

		make depend

	In all subdirectories, a `make depend' is executed. This adds
	the dependency relations of the source files to the Makefiles.

     7) To compile the VCG tool and all demo's, type the command

	 	make xvcg 

	This compiles the program xvcg in src and some small 
	utilities in demo.

     8) To install the binaries and manual pages, type the command

		make install

	In all subdirectories, a `make install' is executed.

     9) To cleanup, type one of the commands

		make clean
		make targetclean
		make veryclean
		make distclean

	Clean means, that all object files and temporary files are 
	deleted.
	Targetclean means, that the binary executables are removed
	from the source directories. This is useful after a call
	of `make install'.
	Veryclean means, that all generated sources are deleted.
	WARNUNG: It may be that you don't have all generators that are 
	needed to regenerate after a veryclean. Thus you should
	avoid veryclean in this case !!!
	Distclean cleans everything unless the files that are in
	the original distribution.

     10) If you want to learn how to use the VCG tool, or if you
	want to test the VCG tool, then run demo.csh in the 
	directory demo.
	Alternatively, you can type

		make test
	or
		make demonstration

	It is useful to make test before make clean.

     11) Typical warnings that may occur during the compilation and
	 installation:

	   a) Unused variables (in drawlib.c, grprint2.c, PSdev.c)
	      occur, because the same code is used several times
	      in these modules, and not all variables are needed
	      each time.
	      I hope that the C optimizer will eliminate this drawback.

	   b) Statement not reached: Here, some assertions are
	      fullfilled such that the unreached statements are not
	      necessary. I sometimes check the system by assertions. 

	   c) Address operand '&' in front of address expressions occur
	      if a function is passed as parameter. Some compiler 
	      require the operand '&' in this case. ANSI compiler
	      should ignore the operand '&', because it is not necessary.

	   d) The messages "not_available  not found", or "bison not found"
	      or "flex not found" or "parsegen not found" during the
	      installation are not fatal.
	      In these cases, one of the preconfigured files is taken
	      instead of the generated one.

	  All these warnings are not fatal and can be ignored !


FURTHER REMARKS 
	In one of the previous distributions, I started to include a 
	version number starting at V.1.0.  Before, I had included a 
	revision number, but this is not appropriate, because it is 
	simply the number of the version control file of main.c.

	Okay: the first release was VCG, no version,  revision 3.4
	The newest release is       VCG, version 1.3, revision 3.17
	I hope nobody is confused by this. 

	If you have successfully tested the tool VCG on a new machine 
	or with a new operating system that I still don't know, please 
	send me the files tMakefile, src/globals.h and demo/demo.csh.
	On problems/warnings, please add a problem description.

	My mail address:   sander@cs.uni-sb.de

	Please read the documentation, the manual pages, and the files
	doc/README*. The documentation is a file "visual.ps" in the 
	directory doc.
	Last minute changes are documented in doc/README*.

	The printer driver and printer utilities in the directory
	demo are written as demonstration, how to deal with PBM files.
	Because there are many printer types in the world, I cannot
	write a printer driver for every type.
	However, many printer drivers are already in the PBM-distribution.


PROBLEMS

     1) The X11 version of VCG has the `InputFocus' problem. 
	This problem depends on the X11 installation and the window
	manager you use.
	After starting and opening the window, the input focus of
	any X11 program is not set to the window, even if the cursor 
	points into the window. As consequence, any input into the
	window is ignored; it is handled as if it would be input
	of the old window that was input focus before starting VCG.
	Nearly all demoprograms of the X11/mit distribution have this
	problem. The user can solve it by moving the cursor one time
	out of the program's window and back into the window. 

      SOLUTION IN THE VCG TOOL:	
	If NOINPUTFOCUS is undefined (see src/globals.h) the VCG tool
	behaves like this:
	The VCG tool tries to solve this problem by setting the input 
	focus actively by XSetInputFocus when startup VCG. This 
	implies that the input focus after a successful start is 
	set correctly to the window the curser points to.
	However, this works only in 99 % of all cases. In some non-
	deterministical cases, the X11 window system notifies a
	`Bad Match Error on XSetInputFocus'. In this case, you should
	define NOINPUTFOCUS when making the tool, or you should use
	the option -grabinputfocus.
	Sorry, but this problem can only be solved by an X11 wizard.
	Hints are wellcome.

     2) (solved)

     3) On color screens, the tool may fail in the startup phase, if
	not enough colors are available. The VCG tool need 32 colors
	and tries to allocate them constructively. If other tools
	(e.g., programs that set the root window to colored background
	pictures) allocate colors destructively, it may happened that
	less than 32 colors are available. 

      SOLUTION 
	Do not use such programs at the same time as the VCG tool
	or call the VCG tool without colors (flag -nocolors).

     4) The default font may not be available. In this case, the VCG 
	tool fails in the startup phase.

      SOLUTION
	Install the default font, or call the VCG tool with an 
	appropriate font (flag -font).

     5) If gcc version 2.4.5 is used, you should not link with
	option -static, because this causes some strange effects.
	For example: xvcg -display something:0.0 does not work
	anymore. Colors are strange, etc.
	This may happen on other versions of gcc, too. 
	Be careful !

      SOLUTION
	Use another version of gcc, another C compiler or don't 
	link statically. The problem does not occur with gcc 2.6.3. 

     6) Some C libraries don't contain certain functions:

	     * alloca is used by bison generated code. 
		(This is one of the small ugly disadvantages of this 
		 excellent parser generator).
	     * getwd is used by the file selector box.
	     * irint is used by the PostScript device driver.

      SOLUTION
	Add certain definition to src/globals.h. Examples:

	#define alloca(x) (malloc(x)) 
	#define getwd(x)  (getcwd(x,1023)) 
	#define irint(x)  ((int)(x)) 

	As an example, see the adaptions I have made for HPUX systems
	at the end of src/globals.h.
	If malloc is used instead of alloca, the parser does not free
	its memory, i.e. more memory is used than necessary.
	If getcwd is used instead of getwd, the file selector box
	is much slower. Hopefully, getcwd is available on the most
	Unix systems.
	If type casts are used instead of irint, the behaviour on
	overflow is not defined anymore. Further, ugly rounding errors
	may occur during the PostScript output.

     7) The bison version 1.16 does not work correctly. If you have used 
	it, the typical message "Unexpected $<start/end of input>$ ..."
	may appear on specifications, that are perfectly correct.

      SOLUTION
	Do not use bison 1.16. Set YACC=not_available in the tMakefile
	instead.
 
     8) The demo sequence that is executed on "make test" does not 
	start the tool vcgdemomaker (message: vcgdemomaker not found,
	... cannot execute).
	This happended on IBM AIX systems.

      SOLUTION
	Try to "make install" before "make test". 
	On SunOs systems, you can do "make test" even without
	"make install", but on IBM AIX systems, this feature does
	not work. I don't know why; I'm not an IBM AIX specialist.

     9) The animation demo at the end of the demo sequence that is
	 executed by "make test" does not work.

       SOLUTION
	 The timing behaviour of your system is different than that
	 of my computer. E.g. if you have discless ELCs connected by
	 NFS, the network may be too slow. By changing some integers 
         in the source of the files demo/animation1.c and 
	 demo/animation2.c would solve the problem.
	 But you will probably not want to change that files.
	 Simply ignore this. Note: the animations are the end of
	 the "make test" show, so you don't miss something important
	 if you do not see them. 

     10) On Solaris (SunOS 5.3), the dynamic libraries of X11 are 
         sometimes not found. Typical messages are something like this:
 
          ld.so.1: xvcg: fatal: libXext.so.0: can't open file: errno=2
          Killed


       SOLUTION
         A user of the VCG tool recommended to add -lsockets to the
         list of libraries, but on our test system, we had no success
         with this.
         In the environment, we set 

         	LD_LIBRARY_PATH=/usr/openwin/lib:/usr/lib:/lib

         Then, we had success and everything was okay.

     11) On Solaris (SunOS 5.3), /bin/install does not exist, thus the
         preconfiguration is wrong (it is for SunOS 4.3.1). 

       SOLUTION
	 We used /usr/ucb/install instead of /bin/install, i.e. in
         tMakefile

           INSTALLDIR = /usr/ucb/install -d -m 755
           INSTALL    = /usr/ucb/install -s -m 755 dummy /home/me/bin/dummy
           INSTALLMAN = /usr/ucb/install -m 644 dummy /home/me/man/manl/dummy

         The real problem is, that both SunOS 4.x and Solaris are reported 
         as 'SunOS' to the Makefile. Thus, the Makefile does not see the
         difference.

     12) On a HP-UX system, I did not succede to compile VCG with
	 X11R4. I don't know why.
	 However, I was successful with X11R5 on that machine.

       SOLUTION
	 Use X11R5 or X11R6 on HP-UX systems.

     13) On a HP-UX system with c89, the library seems to be buggy.
         The declaration of gettimeofday is not found. Further, atof is buggy.

       SOLUTION
         I added a declaration of gettimeofday to src/timelim.c.
         But is is probably superfluous on other (newer) HP-UX systems.
         No solution for the atof problem. Try to avoid floats in your
         specification.
        
     14) On Linux systems, some targets of make like "make clean",
	 "make install" etc. yield a infinite recursion.
	 To my surprise, make install works properly, except the fact that
         it installs infinitely often.
         The reason is probably the incompatible behaviour of gnu-make 3.68.

       SOLUTION
	 Break install by "control-C" after a while.
	 Or install by hand. 

     15) Linux does not have /bin/echo, such that /usr/bin/echo must
	 be used in demo/demo.csh

      SOLUTION
	 Adapt demo/demo.csh by hand, i.e replace all occurences 
	 of /bin/ech by /usr/bin/echo.

     16) After the first animation in the sequence of "make test",
	 the curser may not focus in the main window. The reason is
	 probably some strange behavior of the window manager.
	 The effect is, that you cannot enter anything into the window.
	 This occured on Linux with tvtwm.

       SOLUTION
	 Touch into the main window by the mouse. Then, the window becomes
	 active again, such that you can continue.
	 
     17) The xvcg may start and open the window, but immediately fail
	 by a bus error. Typically, nothing is drawn in the window,
	 and the mouse cursor image has not changed yet.
	 This indicates an incompatible behaviour of X11 during the
	 waiting for X11 Events. It happened on a IBM AIX and on a 
	 DecStation (ULTRIX).

       SOLUTION
	 Add -DAIX or -DULTRIX to the list of CFLAG's in the file 
	 tMakefile. This is already done, if you use a preconfiguration 
	 on a IBM AIX or on ULTRIX.
	 Now, make clean and recompile. 


AUTHORS
     Georg Sander, University of Saarland, 66041 Saarbruecken, Germany.
     Iris Lemke,   University of Saarland, 66041 Saarbruecken, Germany.
	
