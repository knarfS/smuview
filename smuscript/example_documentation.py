import smuview
import sys
import pydoc

# Write pydoc text file
file_name = './smuview_python_bindings.txt'
file = open(file_name, 'w')
sys.stdout = file
pydoc.help(smuview)
file.close()
sys.stdout = sys.__stdout__

# Write pydoc html file to current directory
pydoc.writedoc(smuview)
