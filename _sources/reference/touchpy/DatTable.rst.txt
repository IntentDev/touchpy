touchpy.DatTable
================

.. py:class:: touchpy.DatTable
              DatTable(num_rows: int, num_cols: int)
              DatTable(values: collections.abc.Sequence[str], num_rows: int, num_cols: int)
              DatTable(values: collections.abc.Sequence[collections.abc.Sequence[str]])

   Represents a 2D table of strings.

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

   .. method:: row(int) -> list[str]

      Returns a list of values from the row matching the index.

   .. method:: col(int) -> list[str]

      Returns a list of values from the column matching the index.

   .. method:: cell(int, int) -> str

      Returns the value at the row and column index.

   .. method:: resize(int, int)

      Resizes the table to the specified number of rows and columns.

   .. method:: set_num_rows(int)

      Sets the number of rows in the table.

   .. method:: set_num_cols(int)

      Sets the number of columns in the table.

   .. method:: set_cell(int, int, str)

      Sets the value at the row and column index.

   .. method:: set_row(int, list[str])

      Sets the values of the row at the index.

   .. method:: set_col(int, list[str])

      Sets the values of the column at the index.

   .. method:: append_row(list[str])

      Appends a row to the table.

   .. method:: append_col(list[str])

      Appends a column to the table.

   .. method:: insert_row(int, list[str])

      Inserts a row at the index.

   .. method:: insert_col(int, list[str])

      Inserts a column at the index.

   .. method:: remove_row(int)

      Removes the row at the index.

   .. method:: remove_col(int)

      Removes the column at the index.

   .. method:: clear()

      Removes all rows and columns from the table.

   .. method:: as_list() -> list[list[str]]

      Returns the table as a list of lists.

   .. method:: from_list(list[list[str]], bool) -> DatTable

      Fills the table from a list of lists.

   .. method:: as_table() -> DatTable

      Returns the Out DAT as touchpy.DatTable object

   .. method:: as_string() -> str

      Returns the Out DAT in string format.
      

   
   

   .. rubric:: Attributes

   .. py:property:: num_cols
      :type: int

      (set, get) The number of columns in the table.

   .. py:property:: num_rows
      :type: int

      (set, get) The number of rows in the table.


   
   
   .. rubric:: Methods
   
   .. py:method:: append_col(col: collections.abc.Sequence[str] = []) -> None

      Appends a column to the table.

      Lists smaller or larger than the number of rows will be padded or truncated respectively.

      :param col: the values to append (optional)
      :type col: :py:class:`list`


   .. py:method:: append_row(row: collections.abc.Sequence[str] = []) -> None

      Appends a row to the table.

      Lists smaller or larger than the number of columns will be padded or truncated respectively.

      :param row: the values to append (optional)
      :type row: :py:class:`list`


   .. py:method:: as_list() -> list[list[str]]

      Returns the table as a list of lists.


   .. py:method:: as_string() -> str

      Returns the Out DAT in string format.


   .. py:method:: cell(row: int, col: int) -> str
                  cell(row_name: str, col: int) -> str
                  cell(row: int, col_name: str) -> str
                  cell(row_name: str, col_name: str) -> str

      Returns the value at the row and column index.

      :param row: the index of the row
      :type row: :py:class:`int`
      :param col: the index of the column
      :type col: :py:class:`int`


   .. py:method:: clear() -> None

      Removes all rows and columns from the table.


   .. py:method:: col(index: int) -> list[str]
                  col(name: str) -> list[str]

      Returns a list of values from the column matching the index.

      :param index: the index of the column to return
      :type index: :py:class:`int`


   .. py:method:: from_list(list: DatTable.from_list.list, cast: bool = False) -> None

      Fills the table from a list of lists.

      :param list: the list of lists to fill the table from
      :type list: :py:class:`list`
      :param cast: if True, casts the values to strings (optional)
      :type cast: :py:class:`bool`


   .. py:method:: insert_col(j: int, col: collections.abc.Sequence[str] = []) -> None

      Inserts a column at the index.

      Lists smaller or larger than the number of rows will be padded or truncated respectively.

      :param i: the index to insert the column
      :type i: :py:class:`int`
      :param col: the values to insert (optional)
      :type col: :py:class:`list`


   .. py:method:: insert_row(i: int, row: collections.abc.Sequence[str] = []) -> None

      Inserts a row at the index.

      Lists smaller or larger than the number of columns will be padded or truncated respectively.

      :param i: the index to insert the row
      :type i: :py:class:`int`
      :param row: the values to insert (optional)
      :type row: :py:class:`list`


   .. py:method:: remove_col(j: int) -> None
                  remove_col(name: str) -> None

      Removes the column at the index.

      :param i: the index of the column to remove
      :type i: :py:class:`int`


   .. py:method:: remove_row(i: int) -> None
                  remove_row(name: str) -> None

      Removes the row at the index.

      :param i: the index of the row to remove
      :type i: :py:class:`int`


   .. py:method:: resize(num_rows: int, num_cols: int) -> None

      Resizes the table to the specified number of rows and columns.

      :param numRows: the number of rows
      :type numRows: :py:class:`int`
      :param numCols: the number of columns
      :type numCols: :py:class:`int`


   .. py:method:: row(index: int) -> list[str]
                  row(name: str) -> list[str]

      Returns a list of values from the row matching the index.

      :param index: the index of the row to return
      :type index: :py:class:`int`


   .. py:method:: set_cell(i: int, j: int, value: str) -> None
                  set_cell(rowName: str, j: int, value: str) -> None
                  set_cell(i: int, colName: str, value: str) -> None
                  set_cell(rowName: str, colName: str, value: str) -> None

      Sets the value at the row and column index.

      :param row: the index of the row
      :type row: :py:class:`int`
      :param col: the index of the column
      :type col: :py:class:`int`
      :param value: the value to set
      :type value: :py:class:`str`


   .. py:method:: set_col(j: int, col: collections.abc.Sequence[str]) -> None
                  set_col(name: str, col: collections.abc.Sequence[str]) -> None

      Sets the values of the column at the index.

      Lists smaller than the number of rows will be padded with empty strings.
      Lists larger than the number of rows will be truncated.

      :param i: the index of the column
      :type i: :py:class:`int`
      :param col: the values to set
      :type col: :py:class:`list`


   .. py:method:: set_num_cols(num_cols: int) -> None

      Sets the number of columns in the table.

      :param numCols: the number of columns
      :type numCols: :py:class:`int`


   .. py:method:: set_num_rows(num_rows: int) -> None

      Sets the number of rows in the table.

      :param numRows: the number of rows
      :type numRows: :py:class:`int`


   .. py:method:: set_row(i: int, row: collections.abc.Sequence[str]) -> None
                  set_row(name: str, row: collections.abc.Sequence[str]) -> None

      Sets the values of the row at the index.

      Lists smaller than the number of columns will be padded with empty strings.
      Lists larger than the number of columns will be truncated.

      :param i: the index of the row
      :type i: :py:class:`int`
      :param row: the values to set
      :type row: :py:class:`list`




