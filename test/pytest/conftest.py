#conftest.py

import json
import keyboard
from pathlib import Path
import pytest
import sys

localImportPath = Path(__file__).parents[2] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

def on_frame(comp, notused):
	if (keyboard.is_pressed('q')):
		print("pressed q key")
		comp.stop()
		return
	if comp.start_next_frame():
		comp.stop()
		pass
	else:
		pass


@pytest.fixture(scope="session")
def ref():
	referenceFile = Path(__file__).parents[1] / 'tox/parvalues.json'
	referenceValues = json.loads(referenceFile.read_text(encoding='utf-8'))
	return referenceValues

@pytest.fixture(scope="session")
def comp():
	comp = tp.Comp('../tox/test.tox')
	fluffdata = {}
	comp.set_on_frame_callback(on_frame, fluffdata)
	comp.start()
	yield comp
	print("comp fixture teardown after all tests are complete")
	comp.stop()
	return comp

@pytest.fixture(scope="session")
def stopcomp(comp):
	comp.stop()