import os
import glob

# adjust this to point to your main FreeOrion directory
FOHome = os.path.expanduser('~/programs/freeorion_test/')

#if testAll is true the script will crawl through all FO files, skipping any folders in dirs_to_skip
# checking files but not fixing
testAll = True

# if testAll is false, therefore using file_list, fixfiles controls whether CRLF's will be converted to LF's
fixfiles = False

dirs_to_skip = ['GG', 'OIS', 'PagedGeometry']
exts_to_check = [".py", ".py.*", ".txt",".h",".c",".cpp", ".patch"]

# file_list is a whitespace-separated list of file pathnames (relative to FOHome) to check (and possibly fix) if testAll is false
# the pathnames should NOT have leading slash: for example, 
# use "default/buildings.txt", NOT "/default/buildings.txt"
file_list = "default/AI"

def check_line_endings(filename,  fix=False):
    this_file = open(filename, 'r')
    results={}
    LF=0
    CRLF=0
    CR=0
    nextlines=this_file.readlines(8192)
    while nextlines:
        for thisline in nextlines:
            if thisline[-2:]=='\r\n':
                CRLF+=1
            elif thisline[-1:]=='\n':
                LF+=1
            elif thisline[-1:]=='\r':
                CR+=1
        nextlines=this_file.readlines(8192)
    this_file.close()
    if LF: results['LF']=LF
    if CRLF: results['CRLF']=CRLF
    if ( CRLF or CR ) and fix:
        this_file = open(filename, 'r')
        theselines=this_file.readlines()
        this_file.close()
        this_file = open(filename, 'w')
        for thisline in theselines:
            if thisline[-2:]=='\r\n':
                thisline = thisline[:-2]+'\n'
            elif thisline[-1:]=='\r':
                thisline = thisline[:-1]+'\n'
            this_file.write(thisline)
        this_file.close()
    return results


os.chdir(FOHome)
num_files_checked = 0
if testAll:
    for dirpath, dirnames, filenames in os.walk('.'):
        for skipdir in dirs_to_skip:
            if skipdir in dirnames:
                dirnames.remove(skipdir)
        for fname in filenames:
            for ext in exts_to_check:
                if fname[-len(ext):]==ext:
                    num_files_checked += 1
                    fullpath=os.path.join(dirpath,  fname)
                    results=check_line_endings(fullpath,  fix=fixfiles)
                    results2 = ""
                    if results.keys() not in [[], ['LF']]:
                        if fixfiles:
                            results2=", now are %s"%check_line_endings(fullpath,  fix=False)
                        print fullpath,  ':',  results, results2
    #print "lastpath",  fullpath,  ':',  results


if not testAll:
    print
    files_to_check = []
    for fname in file_list.split():
        if not os.path.isdir(fname):
            files_to_check.append(fname)
        else:
            for path_ext in exts_to_check:
                files_to_check.extend(glob.glob(fname+os.sep+'*'+path_ext))
    for fname in files_to_check:
        num_files_checked += 1
        results1=check_line_endings(fname,  fix=fixfiles)
        if fixfiles and results1.keys()!=['LF']:
            results2=check_line_endings(fname,  fix=False)
            print fname,  "line endings were",  results1,  "now are",  results2
        else:
            print fname,  "line endings are",  results1

print "Done.  Total of %d files checked."%num_files_checked
