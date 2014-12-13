#ifndef SFQ_LIB_H_INCLUDE_ONCE_
#define SFQ_LIB_H_INCLUDE_ONCE_

/*
#pragma GCC diagnostic ignored "-Wunused-label"
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>        /* For mode constants */
#include <assert.h>
#include <stdarg.h>

#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <signal.h>
#include <math.h>

#ifdef WIN32
	#include "win32-dummy-build.h"
#else
	#include <stdbool.h>
	#include <inttypes.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <alloca.h>

	#include <semaphore.h>
	#include <wait.h>
	#include <libgen.h>

	#include <pwd.h>
	#include <grp.h>
	#include <sys/capability.h>
#endif

#include "sfq.h"

/*
uid = 0 は root になって uid_t の初期値には使えないから
uid_t, gid_t はマクロを多用する
*/
#define SFQ_IS_ROOTUID(a)	( (a) == 0)

#define SFQ_UNSET_UID(a)	( (a) = (uid_t)-1 )
#define SFQ_UNSET_GID(a)	( (a) = (gid_t)-1 )

#define SFQ_ISSET_UID(a)	( (a) != (uid_t)-1 )
#define SFQ_ISSET_GID(a)	( (a) != (gid_t)-1 )

#define SFQ_MAX(a, b)		(((a)>(b))?(a):(b))

#define SFQ_LIB_INITIALIZE \
	int fire_line__  = -1; \
	int fire_rc__ = SFQ_RC_UNKNOWN; \
	int fire_errno__ = 0; \
	char fire_reason__[128] = "";


#define SFQ_LIB_CHECKPOINT \
	fire_rc__ = SFQ_RC_SUCCESS; \
SFQ_FAIL_CATCH_LABEL__:


#define SFQ_LIB_IS_FAIL()	(fire_rc__ != SFQ_RC_SUCCESS)
#define SFQ_LIB_IS_SUCCESS()	(fire_rc__ == SFQ_RC_SUCCESS)
#define SFQ_LIB_RC()		(fire_rc__)


#define SFQ_LIB_FINALIZE \
	if (fire_reason__[0]) \
	{ \
		struct tm tm_tmp__; \
		time_t now__; \
		char nowstr__[32] = ""; \
		char fire_errstr__[128] = "-----"; \
		now__ = time(NULL); \
		localtime_r(&now__, &tm_tmp__); \
		strftime(nowstr__, sizeof(nowstr__), "%Y-%m-%d %H:%M:%S", &tm_tmp__); \
		if (fire_errno__) { \
			strerror_r(errno, fire_errstr__, sizeof(fire_errstr__)); \
		} \
		fprintf(stderr, "= %s =\n\t%s(%d)# %s\n\treason=%d [%s]\n\terrno=%d [%s]\n\n", \
			nowstr__, __FILE__, fire_line__, __func__, \
			fire_rc__, fire_reason__, \
			fire_errno__, fire_errstr__); \
	}


#define SFQ_FAIL_SILENT(fire_rc) \
	fire_line__ = __LINE__; \
	fire_rc__ = SFQ_RC_ ## fire_rc; \
	fire_errno__  = errno; \
	goto SFQ_FAIL_CATCH_LABEL__;


#define SFQ_PLUSINTSTR_WIDTH(var)	( ((var) == 0) ? 1 : (((int)log10( (var) )) + 1) )


/*
"fmt, ##__VA_ARGS__" のようにすることで可変長引数部がゼロ個のとき、最後のカンマを除去してくれる
(GCC独自拡張)
*/
#define SFQ_FAIL(fire_rc, fmt, ...) \
	fire_line__ = __LINE__; \
	fire_rc__ = SFQ_RC_ ## fire_rc; \
	fire_errno__  = errno; \
	snprintf(fire_reason__, sizeof(fire_reason__), fmt, ##__VA_ARGS__); \
	goto SFQ_FAIL_CATCH_LABEL__;


/* */
#ifndef SFQ_DEFAULT_QUEUE_DIR
	#define SFQ_DEFAULT_QUEUE_DIR	"/var/tmp"
#endif

#ifndef SFQ_DEFAULT_QUEUE_NAME
	#define SFQ_DEFAULT_QUEUE_NAME	"noname"
#endif

#define SFQ_MAGICSTR			"sfq"

#define SFQ_QUEUE_FILENAME		"data.sfq"
#define SFQ_QUEUE_LOGDIRNAME		"logs"
#define SFQ_QUEUE_PROC_LOGDIRNAME	"proc"
#define SFQ_QUEUE_EXEC_LOGDIRNAME	"exec"

#define SFQ_ALIGN_MARGIN(e)		(((( (e) / 8 ) + (( (e) % 8) ? 1 : 0)) * 8) - (e) )

/* --------------------------------------------------------------
 *
 * 定数定義
 *
 */

/* file open mode */
enum
{
	SFQ_FOM_READ			= 1U,
	SFQ_FOM_WRITE			= 2U,
};

/* proccess info state */
enum
{
	SFQ_PIS_DONE			= 0,
	SFQ_PIS_NONE			= 0,
	SFQ_PIS_WAITFOR,
	SFQ_PIS_LOOPSTART,
	SFQ_PIS_TAKEOUT,
	SFQ_PIS_LOCK,
};

/* take out state */
enum
{
	SFQ_TOS_NONE			= 0,
	SFQ_TOS_SUCCESS,
	SFQ_TOS_APPEXIT_NON0,
	SFQ_TOS_CANTEXEC,
	SFQ_TOS_FAULT,
};

/* --------------------------------------------------------------
 *
 * ファイルに保存される構造体 (8 byte alignment)
 *
 */
struct sfq_process_info
{
	pid_t ppid;			/* 4 */
	pid_t pid;			/* 4 */

	sfq_uchar procstate;		/* 1 */
	sfq_byte filler[7];		/* 7 */

	time_t updtime;			/* 8 */
	ulong start_cnt;		/* 8 */
	ulong loop_cnt;			/* 8 */

	ulong tos_success;		/* 8 */
	ulong tos_appexit_non0;		/* 8 */
	ulong tos_cantexec;		/* 8 */
	ulong tos_fault;		/* 8 */
};

/* 属性ヘッダ */
struct sfq_qh_sval
{
	off_t procseg_start_pos;	/* 8 */
	off_t elmseg_start_pos;		/* 8 */
	off_t elmseg_end_pos;		/* 8 */

	size_t filesize_limit;		/* 8 */
	size_t payloadsize_limit;	/* 8 */
	time_t createtime;		/* 8 */

	ushort procs_num;		/* 2 ... (P) USHRT_MAX _SC_CHILD_MAX */
	sfq_byte filler[6];		/* 6 */
};

struct sfq_qh_dval
{
	off_t elm_last_push_pos;	/* 8 */
	off_t elm_new_push_pos;		/* 8 */
	off_t elm_next_shift_pos;	/* 8 */

	ulong elm_num;			/* 8 */
	ulong elm_lastid;		/* 8 */

	time_t updatetime;		/* 8 */
	ulong update_cnt;		/* 8 */

	char lastoper[4];		/* 4 */
	questate_t questate;		/* 1 */
	sfq_byte filler[3];		/* 3 */
};

struct sfq_q_header
{
	struct sfq_qh_sval sval;
	struct sfq_qh_dval dval;
};

/* ファイルヘッダ */
struct sfq_file_header
{
	char magicstr[4];		/* 4 = "sfq\0" */
	ushort qfh_size;		/* 2 */
	sfq_byte filler[2];		/* 2 */

	struct sfq_q_header qh;

	struct sfq_qh_dval last_qhd1;
	struct sfq_qh_dval last_qhd2;
};

/* 要素ヘッダ */
struct sfq_e_header
{
/* file offset */
	off_t prev_elmpos;		/* 8 */
	off_t next_elmpos;		/* 8 */

/* value */
	ushort eh_size;			/* 2 */
	payload_type_t payload_type;	/* 1 */
	sfq_uchar elmmargin_;		/* 1 ... for debug, set by sfq_copy_val2ioeb() */
	ushort execpath_size;		/* 2 ... (x) USHRT_MAX PATH_MAX */
	ushort metatext_size;		/* 2 ... (m) USHRT_MAX */

	uint execargs_size;		/* 4 ... (a) UINT_MAX _SC_ARG_MAX */
	ushort soutpath_size;		/* 2 ... (o) USHRT_MAX PATH_MAX */
	ushort serrpath_size;		/* 2 ... (e) USHRT_MAX PATH_MAX */

	ulong id;			/* 8 */
	time_t pushtime;		/* 8 */
	uuid_t uuid;			/* 16 */

	size_t payload_size;		/* 8 */

	size_t elmsize_;		/* 8 ... for debug, set by sfq_copy_val2ioeb() */
};

/* --------------------------------------------------------------
 *
 * 関数へのパラメータ構造体
 *
 */

struct sfq_open_names
{
	const char* quename;
	const char* querootdir;

	const char* quedir;
	const char* quefile;
	const char* quelogdir;
	const char* queproclogdir;
	const char* queexeclogdir;
	const char* semname;
};

struct sfq_queue_object
{
	struct sfq_open_names* om;

	mode_t save_umask;
	sem_t* semobj;
	FILE* fp;
	time_t opentime;

	sfq_uchar queue_openmode;
};

struct sfq_ioelm_buff
{
	struct sfq_e_header eh;
	const char* execpath;
	const char* execargs;
	const char* metatext;
	const sfq_byte* payload;
	const char* soutpath;
	const char* serrpath;
};

struct sfq_eloop_params
{
	ushort slotno;

	const char* om_quename;
	const char* om_querootdir;

	const char* om_queproclogdir;
	const char* om_queexeclogdir;

	mode_t dir_perm;
	mode_t file_perm;
};

struct sfq_queue_create_params
{
	const char* quename;
	const char* querootdir;
	uid_t queuserid;
	gid_t quegroupid;
	bool chmod_GaW;
};

/* --------------------------------------------------------------
 *
 * 関数プロトタイプ
 *
 */
extern void sfq_print_sizes(void);
extern void sfq_print_qo(const struct sfq_queue_object* qo);
extern void sfq_print_qf_header(const struct sfq_file_header*);
extern void sfq_print_q_header(const struct sfq_q_header*);
extern void sfq_print_qh_dval(const struct sfq_qh_dval*);
extern void sfq_print_e_header(const struct sfq_e_header*);
extern void sfq_print_procs(const struct sfq_process_info* procs, size_t procs_num);
extern void sfq_close_queue(struct sfq_queue_object* qo);
extern void sfq_qh_init_pos(struct sfq_q_header*);
extern void sfq_free_ioelm_buff(struct sfq_ioelm_buff* ioeb);
extern void sfq_free_open_names(struct sfq_open_names* om);
extern void sfq_reopen_4proc(const char* logdir, ushort slotno, questate_t questate, mode_t file_perm);

extern struct sfq_queue_object* sfq_create_queue(const struct sfq_queue_create_params* qcp);
extern struct sfq_queue_object* sfq_open_queue_rw(const char* querootdir, const char* quename);
extern struct sfq_queue_object* sfq_open_queue_ro(const char* querootdir, const char* quename);
extern struct sfq_open_names* sfq_alloc_open_names(const char* querootdir, const char* quename);

extern int sfq_reserve_proc(struct sfq_process_info* procs, ushort procs_num);

extern bool sfq_writeelm(struct sfq_queue_object* qo, off_t seek_pos, const struct sfq_ioelm_buff* ioeb);
extern bool sfq_readelm(struct sfq_queue_object* qo, off_t seek_pos, struct sfq_ioelm_buff* ioeb);
extern bool sfq_seek_set_and_read(FILE* fp, off_t pos, void* mem, size_t mem_size);
extern bool sfq_seek_set_and_write(FILE* fp, off_t pos, const void* mem, size_t mem_size);
extern bool sfq_go_exec(const char* querootdir, const char* quename, ushort slotno, questate_t questate);
extern bool sfq_copy_ioeb2val(const struct sfq_ioelm_buff* ioeb, struct sfq_value* val);
extern bool sfq_copy_val2ioeb(const struct sfq_value* val, struct sfq_ioelm_buff* ioeb);
extern bool sfq_mkdir_p(const char *arg, mode_t mode);
extern char* sfq_alloc_concat_n(int n, ...);

extern bool sfq_readqfh(struct sfq_queue_object* qo,
	struct sfq_file_header* qfh, struct sfq_process_info** procs_ptr);

extern bool sfq_writeqfh(struct sfq_queue_object* qo, struct sfq_file_header* qfh,
	const struct sfq_process_info* procs, const char* lastoper);

extern void sfq_output_reopen_4exec(FILE* fp, const time_t* now, const char* arg_wpath,
	const char* logdir, const uuid_t uuid, ulong id, const char* ext, const char* env_key,
	mode_t dir_perm, mode_t file_perm);

extern void sfq_write_execrc(const char* logdir, const uuid_t uuid, int rc);

extern int sfq_execwait(const struct sfq_eloop_params* elop, struct sfq_value* val);

#endif

