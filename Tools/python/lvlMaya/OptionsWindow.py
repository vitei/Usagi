import maya.cmds as cmds

class OptionsWindow( object ):
    def __init__(self):
        self.handle = 'vti_OptionsWindow'
        self.title = 'Options Window'
        self.size = (640, 480)
        self.supportsToolAction = False
        self.buttons = []

    def create(self, title=None, labelsAndCommands=None, size=None):
        # Close if exists
        if cmds.window(self.handle, exists=True):
            self.close()

        if not title is None:
            self.title = title

        if not size is None:
            self.size = size

        self.handle = cmds.window(self.handle,
                                  title=self.title,
                                  widthHeight=self.size,
                                  sizeable=False,
                                  menuBar=True)
        
        self.mainForm = cmds.formLayout( nd=100 )
        self.setupCommonMenu()

        stdLabelsAndCommands = (
            ( 'Apply and Close', self.btnActionCmd ),
            ( 'Apply', self.btnApplyCmd ),
            ( 'Close', self.btnCloseCmd )
            )
        if labelsAndCommands is None:
            labelsAndCommands = stdLabelsAndCommands

        self.setupCommonButtons( labelsAndCommands )
        cmds.showWindow()

    def close(self):
        cmds.deleteUI(self.handle, window=True)

    # Menu bar
    def setupCommonMenu(self):
        # Edit menu
        self.editMenu = cmds.menu(label='Edit')
        self.editMenuSave = cmds.menuItem( label='Save Settings', command=self.editMenuSaveCmd )
        self.editMenuReset = cmds.menuItem( label='Reset Settings', command=self.editMenuResetCmd );
        self.editMenuDiv = cmds.menuItem(d=True)    # Divider
        self.editMenuRadio = cmds.radioMenuItemCollection()
        self.editMenuTool = cmds.menuItem(
            label='As Tool',
            radioButton=True,
            enable=self.supportsToolAction,
            command=self.editMenuAsToolCmd )
        self.editMenuAction = cmds.menuItem(
            label='As Action',
            radioButton=True,
            enable=self.supportsToolAction,
            command=self.editMenuAsActionCmd )

        # Help menu
        self.helpMenu = cmds.menu( label='Help')
        self.helpMenuItem = cmds.menuItem( label='Help on %s' % self.title, command=self.helpMenuCmd )

    # Menu commands
    def editMenuSaveCmd( self, *args ): pass
    def editMenuResetCmd( self, *args ): pass
    def editMenuAsToolCmd( self, *args ): pass
    def editMenuAsActionCmd( self, *args ): pass

    def helpMenuCmd(self, *args):
        cmds.launch( web='http://maya-python.com' )

    # Buttons
    def setupCommonButtons(self, labelsAndCommands):
        buttonNum = len( labelsAndCommands )
        buttonHeight = 26
        division = 100
        self.commonButtonSize = ( ( self.size[0] - 18 ) / buttonNum, buttonHeight )

        attachForm = []
        attachPosition = []
        attachControl = []
        attachNone = []
        for i in range(buttonNum):
            newBtn = cmds.button( height=self.commonButtonSize[1], label=labelsAndCommands[i][0], command=labelsAndCommands[i][1] )

            # Attach bottom
            attachForm.append( [newBtn, 'bottom', 5] )

            # Head and tail
            if i == 0:
                attachForm.append( [newBtn, 'left', 5] )
            elif i == buttonNum - 1:
                attachForm.append( [newBtn, 'right', 5] )
            
            # Buttons except head and tail
            if i > 0:
                attachPosition.append( [newBtn, 'left', 1, ( division / buttonNum ) * i] )
                attachControl.append( [self.buttons[i-1], 'right', 4, newBtn] )

            # Top is free
            attachNone.append( [newBtn, 'top'] )
            self.buttons.append( newBtn )

        cmds.formLayout( self.mainForm, e=True,
                        attachForm = attachForm,
                        attachPosition = attachPosition,
                        attachControl = attachControl,
                        attachNone = attachNone)


    # Button commands
    def btnActionCmd( self, *args ):
        self.btnApplyCmd()
        self.btnCloseCmd()

    def btnApplyCmd( self, *args ):
        fileFilter = '*.lvl'
        openPath = self.openFileDialog( fileFilter )

        if not openPath is None:
            print openPath

    def btnCloseCmd( self, *args ):
        self.close()

    # Utilities
    def openFileDialog( self, filter ):
        path = ''
        path = cmds.fileDialog2( fileFilter = filter, fileMode = 1 )
        if isinstance( path, list ): path = path[0]
        return path

    def saveFileDialog( self, filter ):
        path = cmds.fileDialog2( fileMode=0 )
        if not path is None and isinstance( path, list ):
            return path[0]
        else:
            return None
