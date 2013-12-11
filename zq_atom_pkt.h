#ifndef ZQ_ATOM_PKT_H
#define ZQ_ATOM_PKT_H

#ifdef __cplusplus
extern "C" {
#endif // c++

	/* atom struct
	 * 
	 * size
	 * type
	 * data
	 *
	 */

	// ptr
struct zq_atom_header
{
	unsigned int size; // atom pkt data size
	union {
		unsigned int type_i;
		char type_c[4];
	} type;
	// data
};

#ifdef __cplusplus
}
#endif // c++

#endif //  zq_atom_pkt.h

