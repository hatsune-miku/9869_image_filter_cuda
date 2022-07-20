#!ruby
#encoding:utf-8
#
# Another way to checks if two PGM files are the same.
# Version numbers (P2,P5,etc) and comments in the pgm files are ignored.
# Only the actual image data is being compared.
#
# Author: Zhen Guan
# Email: zguan@mun.ca
# Student Number: 202191382
#

puts (["data/output.pgm","data/Expected_output_peppers.ascii.pgm"].map{|f|File.open(f,"r").read.to_s[2..].split("\n").filter{|x|x[0]!='#'}.join.strip.gsub(/\s+/,"")}.inject(:==))?'Success: The output is as expected':'Fail: The output is not as expected'
