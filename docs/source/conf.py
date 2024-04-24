# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html


from pathlib import Path
import sys
localImportPath = Path.cwd().parents[1] / 'install/modules'

if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	



project = 'TouchPy'
copyright = '2024'
author = ''
release = '0.1'


# start autodocsumm version ========================

#extensions = [
#      'sphinx.ext.autodoc', 
#      'sphinx.ext.napoleon', 
#      'autodocsumm', 
#      'sphinx.ext.coverage'
#]

# add in this line for the autosummary functionality
#auto_doc_default_options = {'autoclass_content' : 'both', 'autosummary': True}

# end autodocsumm version ========================

#==== start cupy variant  ========================

extensions = ['sphinx.ext.autodoc',
		  'sphinx.ext.autosummary',
		  'sphinx.ext.napoleon',
		  'sphinx_copybutton']

autosummary_generate = True
# ===== end cupy variant ========================


templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

#html_theme = "pydata_sphinx_theme"
#html_theme = 'sphinx_rtd_theme'
html_theme = "furo"
html_static_path = ['_static']
