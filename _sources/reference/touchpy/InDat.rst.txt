touchpy.InDat
=============

.. py:class:: touchpy.InDat(arg0: TouchObject<TEInstance_>, arg1: TouchObject<TELinkInfo>, /)

   An interface for an InDAT in a loaded TouchDesigner component

   
   


   
   
   .. rubric:: Methods
   
   .. py:method:: from_list(list: InDat.from_list.list, cast: bool = False) -> None

      Fills the table from a list of lists.

      :param list: the list of lists to fill the table from
      :type list: :py:class:`list`
      :param cast: if True, casts the values to strings (optional)
      :type cast: :py:class:`bool`


   .. py:method:: from_string(arg: str, /) -> None

      Fills the In DAT from a string and sets it to Text DAT mode.


   .. py:method:: from_table(arg: DatTable, /) -> None

      Fills the In DAT from a touchpy.DatTable object and sets it to Table DAT mode.




