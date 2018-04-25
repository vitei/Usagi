import sys, os

additionalImportPaths = []
def appendImportPath( path ):
    if not path in additionalImportPaths:
        sys.path.append( path )
        additionalImportPaths.append( path )

def cleanupImportPaths():
    for path in additionalImportPaths:
        sys.path.remove( path )

# LevelEditor library directory
usagi_dir = os.getenv( 'USAGI_DIR' )
appendImportPath( usagi_dir + '/Tools/python/lvl2vhir' )
appendImportPath( usagi_dir + '/Tools/python/lvlMaya' )
appendImportPath( usagi_dir + '/Tools/python/lvlUtil' )

import LevelImportExportWindow
reload( LevelImportExportWindow )

# Dump sys.path
#for p in sys.path: print p

def main():
    win = LevelImportExportWindow.LevelImportExportWindow()
    win.create()

if __name__ == "__main__":
    main()
    cleanupImportPaths()

