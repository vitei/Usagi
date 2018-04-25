import sys, os
import maya.cmds as cmds

import LevelEditor
reload( LevelEditor )
import OptionsWindow
reload( OptionsWindow )

import ImportLevel as importer
reload( importer )
import ExportLevel as exporter
reload( exporter )

class LevelImportExportWindow(OptionsWindow.OptionsWindow):
    def __init__(self):
        return super(LevelImportExportWindow, self).__init__()

    def create(self):
        title = 'Level Import/Export'
        labelsAndCommands = (
            ('Import', self.btnImportCmd),
            ('Export', self.btnExportCmd),
            ('Close', self.btnCloseCmd )
            )
        windowSize = ( 480, 240 )
        return super(LevelImportExportWindow, self).create(title, labelsAndCommands=labelsAndCommands, size=windowSize)

    def btnImportCmd(self, *args):
        fileFilter = '*.lvl'
        openPath = self.openFileDialog( fileFilter )

        # Invalid path
        if openPath is None:
            return

        # Delete old 'lvl' group if exists
        if cmds.objExists( 'lvl' ):
            cmds.select( 'lvl', r=True )
            cmds.delete()

        # Create new one
        cmds.group( empty=True, name='lvl')
        cmds.select( 'lvl', r=True )

        lvl = LevelEditor.Game.Level( openPath )
        importer.importLevel( lvl )

        self.btnCloseCmd()

    def btnExportCmd(self, *args):
        filter = '*.lvl'
        path = self.saveFileDialog( filter )
        if not path is None:
            path = path.replace( '.*', '.lvl' )

            exporter.exportLevel( path )
            self.btnCloseCmd()
