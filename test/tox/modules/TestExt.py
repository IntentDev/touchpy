import json
import TDJSON

class TestExt:
	"""
	This component is used for unit tests.

	Upon init it sets all pars to values it reads from a json.
	"""
	def __init__(self, ownerComp):
		# The component to which this extension is attached
		self.ownerComp = ownerComp
		self.loadParValues()
		
	def loadParValues(self):
		with open('parvalues.json', 'r') as parvalues_file:
			parvalues = json.load(parvalues_file)
			TDJSON.addParametersFromJSONOp(self.ownerComp, parvalues, setValues=True)