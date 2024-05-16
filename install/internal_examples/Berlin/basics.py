import touchpy as tp
comp = tp.Comp('MyComponent.tox')

def on_layout_change(comp, info):
	print('layout changed:')
	print('in tops:', comp.in_tops.names)
	print('out tops:', comp.out_tops.names)
	print('in chops:', comp.in_chops.names)
	print('out chops:', comp.out_chops.names)
	print('in dats:', comp.in_dats.names)
	print('out dats:', comp.out_dats.names)
	print('pars:', comp.par.names)
	
comp.set_on_layout_change_callback(on_layout_change, {})

