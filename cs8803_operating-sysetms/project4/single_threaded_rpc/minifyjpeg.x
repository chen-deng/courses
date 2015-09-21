/*
 * Complete this file and run rpcgen -MN minifyjpeg.x
 */

struct Img {
	opaque buf<>;
};

program MINIFY_PROG {
	version MINIFY_VERS {
		Img MINIFYJPEG(Img) = 1;		
	} = 1;
} = 0x23451111;