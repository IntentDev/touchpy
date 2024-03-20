def test_float(comp, ref):
	parname = "Float"
	print(f"testing {parname}...")
	val = comp.par[parname].val
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_float2(comp, ref):
	parname = "Float2"
	print(f"testing {parname}...")
	val = [comp.par[parname].x, comp.par[parname].y]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_float3(comp, ref):
	parname = "Float3"
	print(f"testing {parname}...")
	val = [comp.par[parname].x, comp.par[parname].y, comp.par[parname].z]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_float4(comp, ref):
	parname = "Float4"
	print(f"testing {parname}...")
	val = [comp.par[parname].x, comp.par[parname].y, comp.par[parname].z, comp.par[parname].w]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_int(comp, ref):
	parname = "Int"
	print(f"testing {parname}...")
	val = comp.par[parname].val
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_int2(comp, ref):
	parname = "Int2"
	print(f"testing {parname}...")
	val = [comp.par[parname].x, comp.par[parname].y]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_int3(comp, ref):
	parname = "Int3"
	print(f"testing {parname}...")
	val = [comp.par[parname].x, comp.par[parname].y, comp.par[parname].z]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_int4(comp, ref):
	parname = "Int4"
	print(f"testing {parname}...")
	val = [comp.par[parname].x, comp.par[parname].y, comp.par[parname].z, comp.par[parname].w]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_rgb(comp, ref):
	parname = "Rgb"
	print(f"testing {parname}...")
	val = [comp.par[parname].r, comp.par[parname].g, comp.par[parname].b]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_rgba(comp, ref):
	parname = "Rgba"
	print(f"testing {parname}...")
	val = [comp.par[parname].r, comp.par[parname].g, comp.par[parname].b, comp.par[parname].a]
	ref = ref["Test"][parname]["val"]
	assert val == ref

def test_string(comp, ref):
	parname = "Str"
	print(f"testing {parname}...")
	val = comp.par[parname].val
	print("val", val)
	ref = ref["Test"][parname]["val"]
	print("ref", ref)
	assert val == ref

def test_stop(comp):
	print(f"testing Stop...")
	comp.stop()
	assert 3 == 3		