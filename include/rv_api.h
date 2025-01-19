
#ifndef _RV_API_H_
#define _RV_API_H_

#include "shape_bits.h"	// dimension_t

#define MAX_DISKS		8
#define BLOCK_SIZE		1024	// BUG?  get from system include?


/* Public prototypes */

/* rawvol.c */
ITEM_INTERFACE_PROTOTYPES(RV_Inode,rv_inode)

#define rv_inode_of(s)	_rv_inode_of(QSP_ARG  s)
#define get_rv_inode(s)	_get_rv_inode(QSP_ARG  s)
#define del_rv_inode(s)	_del_rv_inode(QSP_ARG  s)

extern int 	is_rv_directory(RV_Inode *);
extern int 	is_rv_link(RV_Inode *);
extern int	_rv_truncate(QSP_ARG_DECL  RV_Inode *,dimension_t);
#define rv_truncate(inp,d) _rv_truncate(QSP_ARG  inp,d)
extern int	legal_rv_filename(const char *);
extern void	_traverse_rv_inodes( QSP_ARG_DECL  void (*f)(QSP_ARG_DECL  RV_Inode *) );
#define traverse_rv_inodes(f) _traverse_rv_inodes( QSP_ARG  f)

extern int	ensure_default_rv(SINGLE_QSP_ARG_DECL);
extern int	rv_get_ndisks(void);
extern void	rv_sync(SINGLE_QSP_ARG_DECL);

extern int _remember_frame_info(QSP_ARG_DECL  RV_Inode *inp, int index, USHORT_ARG nerr, dimension_t *frames);
#define remember_frame_info(inp,index,nerr,frames) _remember_frame_info(QSP_ARG  inp,index,nerr,frames)

extern void	_setup_rv_iofile(QSP_ARG_DECL  RV_Inode *inp);
#define setup_rv_iofile(inp) _setup_rv_iofile(QSP_ARG  inp)

extern int32_t	n_rv_disks(void);
extern int	rv_access_allowed(QSP_ARG_DECL  RV_Inode *inp);

extern int is_rv_directory(RV_Inode *inp);
extern int is_rv_link(RV_Inode *inp);
extern int is_rv_movie(RV_Inode *inp);
extern const char * rv_name(RV_Inode *inp);
extern int rv_frames_to_allocate( int min_frames );
extern void set_rv_n_frames(RV_Inode *inp, int32_t n);
extern int rv_movie_extra(RV_Inode *inp);
extern Shape_Info *rv_movie_shape(RV_Inode *inp);
extern RV_Inode *rv_inode_alloc(void);

extern void	_rv_info(QSP_ARG_DECL  RV_Inode *);
extern int	_creat_rv_file(QSP_ARG_DECL  const char *,dimension_t,int *);
extern int	_queue_rv_file(QSP_ARG_DECL  RV_Inode *,int *);
extern int	_rv_rmfile(QSP_ARG_DECL  const char *name);
extern int	_rv_set_shape(QSP_ARG_DECL  const char *filename, Shape_Info *shpp);
extern int	_rv_realloc(QSP_ARG_DECL  const char *,dimension_t);
extern int	_rv_frame_seek(QSP_ARG_DECL  RV_Inode *,dimension_t);

#define rv_info(inp) _rv_info(QSP_ARG  inp)
#define creat_rv_file(s,n,p) _creat_rv_file(QSP_ARG  s,n,p)
#define queue_rv_file(inp,n) _queue_rv_file(QSP_ARG  inp,n)
#define rv_rmfile(name) _rv_rmfile(QSP_ARG  name)
#define rv_set_shape(filename,shpp) _rv_set_shape(QSP_ARG  filename,shpp)
#define rv_realloc(buf,n) _rv_realloc(QSP_ARG  buf,n)
#define rv_frame_seek(inp,n) _rv_frame_seek(QSP_ARG  inp,n)

void _make_movie_from_inode(QSP_ARG_DECL  RV_Inode *inp);
#define make_movie_from_inode(inp) _make_movie_from_inode(QSP_ARG  inp)
extern void	_update_movie_database(QSP_ARG_DECL  RV_Inode *inp);
#define update_movie_database(inp) _update_movie_database(QSP_ARG  inp)
extern void _init_rv_movies(SINGLE_QSP_ARG_DECL);
#define init_rv_movies() _init_rv_movies(SINGLE_QSP_ARG)

// rawvol_player.c
extern void _play_rawvol_movie(QSP_ARG_DECL  Image_File *ifp);
#define play_rawvol_movie(ifp) _play_rawvol_movie(QSP_ARG  ifp)
extern void _play_rawvol_frame(QSP_ARG_DECL  Image_File *ifp, uint32_t frame);
#define play_rawvol_frame(ifp, frame) _play_rawvol_frame(QSP_ARG  ifp, frame)
extern void _init_rawvol_viewer(QSP_ARG_DECL  int width, int height, int depth);
#define init_rawvol_viewer(width, height, depth) _init_rawvol_viewer(QSP_ARG  width, height, depth)
extern void display_to_rawvol_viewer(Data_Obj *dp);

#ifndef BUILD_FOR_OBJC
extern Viewer *rawvol_viewer(void);
#endif // BUILD_FOR_OBJC


#endif /* undef _RV_API_H_ */

