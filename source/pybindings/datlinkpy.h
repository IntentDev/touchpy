#pragma once


constexpr static char DatTableDoc[] =
R"(Represents a 2D table of strings.

Constructor 1:
	Default constructor, creates an empty DatTable object.

Constructor 2:
	Args:
		values (list[str]): A single list fill the table with (length must be == num_rows * num_cols).
		num_rows (int): The number of rows in the table.
		num_cols (int): The number of columns in the table.

Constructor 3:
	Args:
		values (list[list[str]]): A list of lists to fill the table with.

Properties:
	num_rows (int): (set, get) The number of rows in the table.
	num_cols (int): (set, get) The number of columns in the table.

Methods:
	row(int) -> list[str]: Returns a list of values from the row matching the index.
	col(int) -> list[str]: Returns a list of values from the column matching the index.
	cell(int, int) -> str: Returns the value at the row and column index.
	resize(int, int): Resizes the table to the specified number of rows and columns.
	set_num_rows(int): Sets the number of rows in the table.
	set_num_cols(int): Sets the number of columns in the table.
	set_cell(int, int, str): Sets the value at the row and column index.
	set_row(int, list[str]): Sets the values of the row at the index.
	set_col(int, list[str]): Sets the values of the column at the index.
	append_row(list[str]): Appends a row to the table.
	append_col(list[str]): Appends a column to the table.
	insert_row(int, list[str]): Inserts a row at the index.
	insert_col(int, list[str]): Inserts a column at the index.
	remove_row(int): Removes the row at the index.
	remove_col(int): Removes the column at the index.
	clear(): Removes all rows and columns from the table.
	as_list() -> list[list[str]]: Returns the table as a list of lists.
	from_list(list[list[str]], bool) -> DatTable: Fills the table from a list of lists.
	as_table() -> DatTable: Returns the Out DAT as touchpy.DatTable object
	as_string() -> str: Returns the Out DAT in string format.

)";

constexpr static char num_rowsDoc[] =
R"((set, get) The number of rows in the table.
)";

constexpr static char num_colsDoc[] =
R"((set, get) The number of columns in the table.
)";

constexpr static char rowDoc[] =
R"(Returns a list of values from the row matching the index.

Args:
	index (int) : the index of the row to return
)";


constexpr static char colDoc[] =
R"(Returns a list of values from the column matching the index.

Args:
	index (int) : the index of the column to return
)";

constexpr static char cellDoc[] =
R"(Returns the value at the row and column index.

Args:
	row (int) : the index of the row
	col (int) : the index of the column
)";

constexpr static char reseizeDoc[] =
R"(Resizes the table to the specified number of rows and columns.

Args:
	numRows (int) : the number of rows
	numCols (int) : the number of columns
)";

constexpr static char set_num_rowsDoc[] =
R"(Sets the number of rows in the table.

Args:
	numRows (int) : the number of rows
)";

constexpr static char set_num_colsDoc[] =
R"(Sets the number of columns in the table.

Args:
	numCols (int) : the number of columns
)";

constexpr static char set_cellDoc[] =
R"(Sets the value at the row and column index.

Args:
	row (int) : the index of the row
	col (int) : the index of the column
	value (str) : the value to set
)";

constexpr static char set_rowDoc[] =
R"(Sets the values of the row at the index.

Lists smaller than the number of columns will be padded with empty strings.
Lists larger than the number of columns will be truncated.

Args:
	i (int) : the index of the row
	row (list) : the values to set 
)";

constexpr static char set_colDoc[] =
R"(Sets the values of the column at the index.

Lists smaller than the number of rows will be padded with empty strings.
Lists larger than the number of rows will be truncated.

Args:
	i (int) : the index of the column
	col (list) : the values to set 
)";

constexpr static char append_rowDoc[] =
R"(Appends a row to the table.

Lists smaller or larger than the number of columns will be padded or truncated respectively.

Args:
	row (list) : the values to append (optional)
)";

constexpr static char append_colDoc[] =
R"(Appends a column to the table.

Lists smaller or larger than the number of rows will be padded or truncated respectively.

Args:
	col (list) : the values to append (optional)
)";

constexpr static char insert_rowDoc[] =
R"(Inserts a row at the index.

Lists smaller or larger than the number of columns will be padded or truncated respectively.

Args:
	i (int) : the index to insert the row
	row (list) : the values to insert (optional)
)";

constexpr static char insert_colDoc[] =
R"(Inserts a column at the index.

Lists smaller or larger than the number of rows will be padded or truncated respectively.

Args:
	i (int) : the index to insert the column
	col (list) : the values to insert (optional)
)";

constexpr static char remove_rowDoc[] =
R"(Removes the row at the index.

Args:
	i (int) : the index of the row to remove
)";

constexpr static char remove_colDoc[] =
R"(Removes the column at the index.

Args:
	i (int) : the index of the column to remove
)";

constexpr static char clearDoc[] =
R"(Removes all rows and columns from the table.
)";

constexpr static char as_listDoc[] =
R"(Returns the table as a list of lists.
)";

constexpr static char from_listDoc[] =
R"(Fills the table from a list of lists.

Args:
	list (list) : the list of lists to fill the table from
	cast (bool) : if True, casts the values to strings (optional)
)";

constexpr static char as_tableDoc[] =
R"(Returns the Out DAT as touchpy.DatTable object
)";

constexpr static char as_stringDoc[] =
R"(Returns the Out DAT in string format.
)";

constexpr static char countDocOutDat[] =
R"(Returns the number of Out DATs in the loaded tox.
)";

constexpr static char namesDocOutDat[] =
R"(Returns a list of names of all Out DATs in the loaded tox.
)";

constexpr static char from_tableDoc[] =
R"(Fills the In DAT from a touchpy.DatTable object and sets it to Table DAT mode.
)";

constexpr static char from_stringDoc[] =
R"(Fills the In DAT from a string and sets it to Text DAT mode.
)";

constexpr static char countDocInDat[] =
R"(Returns the number of In DATs in the loaded tox.
)";

constexpr static char namesDocInDat[] =
R"(Returns a list of names of all In DATs in the loaded tox.
)";

