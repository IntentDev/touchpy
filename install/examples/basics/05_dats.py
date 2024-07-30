import touchpy as tp
import utils
import numpy as np

class MyComp (tp.Comp):
	def __init__(self):
		super().__init__()
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

		self.frame = 0

		# create a new DatTable object with 3 rows and 3 columns
		self.my_table = tp.DatTable(2, 3)
		self.my_table.set_row(0, ['a', 'b', 'c'])
		self.my_table.set_row(1, ['1', '2', '3'])
		self.my_table.append_row(['d', 'e', 'f'])
		self.my_table.append_col(['4', '5', '6'])

		self.my_table2  = tp.DatTable()
		values = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
		# tables can be created from a list of lists and can only store strings
		# use the cast flag to convert the values to the string representation
		self.my_table2.from_list(values, cast=True)

		print(
			'my_table2 as list:\n', 
			*[f"\t{self.my_table2.row(i)}\n" for i in range(self.my_table2.num_rows)]
		)



	def on_layout_change(self, info):
		print('layout changed:')
		print('out dats:\n', *[f"\t{name}\n" for name in self.out_dats.names])
		print('in dats:\n', *[f"\t{name}\n" for name in self.in_dats.names])

		if 'Openwindow' in self.par.names:
			# only works on supported TD builds
			# official build with support should be available soon
			# self.par['Openwindow'].pulse()
			pass

	def on_frame(self, info):
		if utils.check_key('q'):
			self.stop()
			return

		# all "out" data retrieved from a component (from out TOPs, CHOPs, DATs) must be done 
		# before calling start_next_frame() (pars are an exception)

		out_dat1 = self.out_dats[0]
		out_dat1_table = out_dat1.as_table()
		out_dat1_str = out_dat1.as_string()

		out_dat2 = self.out_dats['datOut2']
		out_dat2_table = out_dat2.as_table()
		out_dat2_str = out_dat2.as_string()

		if (self.frame == 60):
			print('out_dat1 num rows:', out_dat1_table.num_rows)
			print('out_dat1 num cols:', out_dat1_table.num_cols, '\n')

			print('out_dat1 as string:\n', out_dat1_str, '\n')
			
			print('out_dat1 rows:\n', *[f"\t{row}\n" for row in out_dat1_table.as_list()])

			print('out_dat1 1, 1:', out_dat1_table[2, 2], '\n')
			print('out_dat1 3, "col3":', out_dat1_table[2, 'col2'], '\n')
			print('out_dat1 "row3", 1:', out_dat1_table.cell('row2', 1), '\n')


			print('out_dat2 rows:\n', *[f"\t{row}\n" for row in out_dat2_table.as_list()])
			print('out_dat2 as string:', out_dat2_str, '\n')


		self.in_dats[0].from_string(f"Hello World! frame: {self.frame}")

		self.in_dats['datIn2'].from_table(self.my_table)




		self.start_next_frame()

		self.frame += 1



comp = MyComp()
comp.load('TopChopDatIO.tox') 
comp.start()
comp.unload()






