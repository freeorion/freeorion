import os
import glob

# adjust this to point to your main FreeOrion directory
FOHome = os.path.expanduser('~/programs/freeorion_test/')

#if testAll is true the script will crawl through all FO files, skipping any folders in dirs_to_skip
# checking files but not fixing
testAll = False
dirs_to_skip = ['GG', 'OIS']

# file_list is a whitespace-separated list of file pathnames (relative to FOHome) to check (and possibly fix) if testAll is false
# the pathnames should NOT have leading slash: for example, 
# use "default/buildings.txt", NOT "/default/buildings.txt"
file_list = "default/AI"

# if testAll is false, therefore using file_list, fixfiles controls whether CRLF's will be converted to LF's
fixfiles = True

def checkLineEndings(filename,  fix=False):
    thisFile = open(filename, 'r')
    results={}
    LF=0
    CRLF=0
    CR=0
    nextlines=thisFile.readlines(8192)
    while nextlines:
        for thisline in nextlines:
            if thisline[-2:]=='\r\n':
                CRLF+=1
            elif thisline[-1:]=='\n':
                LF+=1
            elif thisline[-1:]=='\r':
                CR+=1
        nextlines=thisFile.readlines(8192)
    thisFile.close()
    if LF: results['LF']=LF
    if CRLF: results['CRLF']=CRLF
    if ( CRLF or CR ) and fix:
        thisFile = open(filename, 'r')
        theselines=thisFile.readlines()
        thisFile.close()
        thisFile = open(filename, 'w')
        for thisline in theselines:
            if thisline[-2:]=='\r\n':
                thisline = thisline[:-2]+'\n'
            elif thisline[-1:]=='\r':
                thisline = thisline[:-1]+'\n'
            thisFile.write(thisline)
        thisFile.close()
    return results


os.chdir(FOHome)
if testAll:
    for dirpath, dirnames, filenames in os.walk('.'):
        for skipdir in dirs_to_skip:
            if skipdir in dirnames:
                dirnames.remove(skipdir)
        for fname in filenames:
            for ext in ['.h', '.cpp', '.py']:
                if fname[-len(ext):]==ext:
                    fullpath=os.path.join(dirpath,  fname)
                    results=checkLineEndings(fullpath,  fix=False)
                    if results.keys()!=['LF']:
                        print fullpath,  ':',  results
    #print "lastpath",  fullpath,  ':',  results


if not testAll:
    print
    files_to_check = []
    for fname in file_list.split():
        if not os.path.isdir(fname):
            files_to_check.append(fname)
        else:
            for path_ext in [".py",".txt",".h",".c",".cpp"]:
                files_to_check.extend(glob.glob(fname+os.sep+'*'+path_ext))
    for fname in files_to_check:
        results1=checkLineEndings(fname,  fix=fixfiles)
        if fixfiles and results1.keys()!=['LF']:
            results2=checkLineEndings(fname,  fix=False)
            print fname,  "line endings were",  results1,  "now are",  results2
        else:
            print fname,  "line endings are",  results1
