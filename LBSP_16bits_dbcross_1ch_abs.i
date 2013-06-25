// note: this is the LBSP 16 bit double-cross single channel pattern as used in
// the original article by G.-A. Bilodeau et al.
// 
//  O   O   O          4 ..  3 ..  6
//    O O O           .. 15  8 13 ..
//  O O X O O    =>    0  9  X 11  1
//    O O O           .. 12 10 14 ..
//  O   O   O          7 ..  2 ..  5
//
// must be defined externally:
//		_t				(uchar, absolute threshold used for comparisons)
//		_data			(uchar*, single-channel data to be covered by the pattern)
//		_refdata		(uchar*, single-channel data to be used for comparisons)
//		_y				(int, pattern rows location in the image data)
//		_x				(int, pattern cols location in the image data)
//		_step_row		(int, step size between rows, including padding)
//		_res			(ushort, 16 bit result vector)
//		absdiff_uchar	(function, returns the absolute difference between two uchars)

#ifdef _val
#error "definitions clash detected"
#else
#define _val(x,y) _data[_step_row*(_y+y)+_x+x]
#endif

const uchar _ref = _refdata[_step_row*(_y)+_x];
_res= ((absdiff_uchar(_val(-1, 1),_ref) > _t) << 15)
	+ ((absdiff_uchar(_val( 1,-1),_ref) > _t) << 14)
	+ ((absdiff_uchar(_val( 1, 1),_ref) > _t) << 13)
	+ ((absdiff_uchar(_val(-1,-1),_ref) > _t) << 12)
	+ ((absdiff_uchar(_val( 1, 0),_ref) > _t) << 11)
	+ ((absdiff_uchar(_val( 0,-1),_ref) > _t) << 10)
	+ ((absdiff_uchar(_val(-1, 0),_ref) > _t) << 9)
	+ ((absdiff_uchar(_val( 0, 1),_ref) > _t) << 8)
	+ ((absdiff_uchar(_val(-2,-2),_ref) > _t) << 7)
	+ ((absdiff_uchar(_val( 2, 2),_ref) > _t) << 6)
	+ ((absdiff_uchar(_val( 2,-2),_ref) > _t) << 5)
	+ ((absdiff_uchar(_val(-2, 2),_ref) > _t) << 4)
	+ ((absdiff_uchar(_val( 0, 2),_ref) > _t) << 3)
	+ ((absdiff_uchar(_val( 0,-2),_ref) > _t) << 2)
	+ ((absdiff_uchar(_val( 2, 0),_ref) > _t) << 1)
	+ ((absdiff_uchar(_val(-2, 0),_ref) > _t));

#undef _val
		