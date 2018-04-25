import os, sys, re
import xml.etree.ElementTree as etree
import xml.dom.minidom as minidom

import LevelEditor


def saveLevelXml( path, lvl ):
    # Print prettified XML
    rough_string = etree.tostring( lvl.root, 'utf-8' )
    reparsed = minidom.parseString( rough_string )
    prettified = reparsed.toprettyxml('\t', '\n', 'utf-8')

    # 'toprettyxml' doesn't work against pre-prettified XML. Then delete annoying blank rows manually.
    prettified = re.sub( r'(?m)^[\t]+\n', '', prettified )

    f1 = open(path, 'w')
    f1.write( prettified )
    f1.close()