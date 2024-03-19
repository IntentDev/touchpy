import json
import TDJSON


class TestExt:
	"""
	TestExt description
	"""
	def __init__(self, ownerComp):
		# The component to which this extension is attached
		self.ownerComp = ownerComp
		self.loadParValues()
		
	def loadParValues(self):
		with open('parvalues.json', 'r') as parvalues_file:
			parvalues = json.load(parvalues_file)
			TDJSON.addParametersFromJSONOp(self.ownerComp, parvalues, setValues=True)