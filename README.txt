/** 
  @mainpage 
 
  @section intro_sec Introduction
 
    This project allows any data file to be hidden into a 24-bit bitmap.
    Each byte of the data file is split amongst a pixel in the bitmap. Each 
    colour component (Red, Green and Blue) shares 2-3 bits in their least
    significant bytes. Information can be encoded and decoded with this via
    arguments at execution.
 
  @section usage_sec Usage
 
    To encode a file pass in the bitmap to copy and the file containing the 
    desired information to hide.
    <CODE> \n
    ./executable <bitmap_to_duplicate> <data> 
    \n </CODE>
    To decode a file just pass in the bitmap containing the hidden information. 
    <CODE> \n
    ./executable <bitmap_with_info> 
    \n </CODE>

  @section li_sec License
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
