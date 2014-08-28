#pragma once

// modified from LZSS.C

/**************************************************************
	LZSS.C -- A Data Compression Program
	(tab = 4 spaces)
***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************/

class IFile;

class LZSS{
private:
	static const int N = 4096; /* size of ring buffer */
	static const int F = 18; /* upper limit for match_length */
	static const int THRESHOLD = 2;   /* encode string into position and length
									  if match_length is greater than this */
	static const int NIL = N;	/* index for root of binary search trees */
private:
	unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
								with extra F-1 bytes to facilitate string comparison */
	int	match_position, match_length;  /* of longest match.  These are
										   set by the InsertNode() procedure. */
	int	lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
												 parents -- These constitute binary search trees. */
public:
	IFile *infile, *outfile;  /* input & output files */
private:
	void InitTree(void);
	void InsertNode(int r);
	void DeleteNode(int p);
public:
	void Encode(void);
	void Decode(void);
};
