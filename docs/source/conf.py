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

#extensions = ['sphinx.ext.autodoc',
#		  'sphinx.ext.autosummary',
#		  'sphinx.ext.napoleon',
#		  'sphinx_copybutton']

#autosummary_generate = True

#autodoc_member_order = 'groupwise'
#autodoc_typehints = 'both'


#autodoc_default_options = {
#    'members': True,
#    'member-order': 'groupwise',
#    'special-members': '__init__',
#    'undoc-members': True,
#    'exclude-members': '__weakref__'
#}


# ===== end cupy variant ========================

#==== start autoapi variant ========================
# sewe https://bylr.info/articles/2022/05/10/api-doc-with-sphinx-autoapi/

extensions = ['sphinx.ext.autodoc',
		  'sphinx.ext.napoleon',
		  'autoapi.extension',
          "sphinx.ext.intersphinx",
          'numpydoc']

autoapi_dirs = ['../../out/build/x64-release']
autoapi_type = "python"

templates_path = ['_templates']
exclude_patterns = []
autoapi_template_dir = "_templates/autoapi"

autoapi_keep_files = True
autodoc_typehints = "signature"
autoapi_own_page_level = "class"
autoapi_root = 'reference'
autoapi_add_toctree_entry = False
maximum_signature_line_length = 80

rst_prolog = """
.. role:: summarylabel
"""

html_css_files = [
    "css/touchpy.css",
]

def contains(seq, item):
    return item in seq

def prepare_jinja_env(jinja_env) -> None:
    jinja_env.tests["contains"] = contains

autoapi_prepare_jinja_env = prepare_jinja_env

# napoleon options
#napoleon_use_admonition_for_notes = True
napoleon_preprocess_types = True
napoleon_type_aliases = {
    "Callable": "collections.abc.Callable"
}



# -- Intersphinx options
intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "numpy": ("https://numpy.org/doc/stable/", None)
}


#===== END autoapi variant ========================





# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

#html_theme = "pydata_sphinx_theme"
#html_theme = 'sphinx_rtd_theme'
html_theme = "furo"
html_static_path = ['_static']
