#include "sfq-lib.h"

int sfq_pop(const char* querootdir, const char* quename, struct sfq_value* val)
{
SFQ_ENTP_ENTER

	struct sfq_queue_object* qo = NULL;

	sfq_bool b = SFQ_false;
	size_t eh_size = 0;
	off_t elm_pos = 0;

	struct sfq_ioelm_buff ioeb;
	struct sfq_file_header qfh;

/* initialize */
	eh_size = sizeof(struct sfq_e_header);

	bzero(&qfh, sizeof(qfh));
	bzero(&ioeb, sizeof(ioeb));

/* check argument */
	if (! val)
	{
		SFQ_FAIL(EA_FUNCARG, "val is null");
	}

/* open queue-file */
	qo = sfq_open_queue_rw(querootdir, quename);
	if (! qo)
	{
		SFQ_FAIL(EA_OPENQUEUE, "sfq_open_queue_rw");
	}

/* read file-header */
	b = sfq_readqfh(qo, &qfh, NULL);
	if (! b)
	{
		SFQ_FAIL(EA_READQFH, "sfq_readqfh");
	}

/* check accept state */
	if (! (qfh.qh.dval.questate & SFQ_QST_TAKEOUT_ON))
	{
		SFQ_FAIL_SILENT(W_TAKEOUT_STOPPED);
	}

/* check empty */
	if (qfh.qh.dval.elm_num == 0)
	{
		SFQ_FAIL_SILENT(W_NOELEMENT);
	}

#ifdef SFQ_DEBUG_BUILD
	assert(qfh.qh.dval.elm_next_shift_pos);
#endif
	if (qfh.qh.dval.elm_last_push_pos == 0)
	{
		SFQ_FAIL(EA_ASSERT, "qfh.qh.dval.elm_last_push_pos == 0");
	}

	elm_pos = qfh.qh.dval.elm_last_push_pos;

/* read element */
	b = sfq_readelm(qo, elm_pos, &ioeb);
	if (! b)
	{
		SFQ_FAIL(EA_RWELEMENT, "sfq_readelm");
	}

#ifdef SFQ_DEBUG_BUILD
	sfq_print_e_header(&ioeb.eh);
#endif

/* update queue file header */
	qfh.qh.dval.elm_num--;

	if (qfh.qh.dval.elm_num == 0)
	{
	/* 要素がなくなったらヘッダのポジションを初期化する */

		sfq_qh_init_pos(&qfh.qh);
	}
	else
	{
/* update next-element-prev_elmpos */
		struct sfq_e_header prev_eh;

		b = sfq_seek_set_and_read(qo->fp, ioeb.eh.prev_elmpos, &prev_eh, eh_size);
		if (! b)
		{
			SFQ_FAIL(EA_SEEKSETIO, "sfq_seek_set_and_read(prev_eh)");
		}

		/* 前の要素の next_elmpos に 0 を設定し、リンクを切る */
		prev_eh.next_elmpos = 0;

		b = sfq_seek_set_and_write(qo->fp, ioeb.eh.prev_elmpos, &prev_eh, eh_size);
		if (! b)
		{
			SFQ_FAIL(EA_SEEKSETIO, "sfq_seek_set_and_write(prev_eh)");
		}

/* update next shift pos */
		qfh.qh.dval.elm_last_push_pos = ioeb.eh.prev_elmpos;
		qfh.qh.dval.elm_new_push_pos = elm_pos;
	}

/* set val */
	b = sfq_copy_ioeb2val(&ioeb, val);
	if (! b)
	{
		SFQ_FAIL(EA_COPYVALUE, "sfq_copy_ioeb2val");
	}

/* update header */
	b = sfq_writeqfh(qo, &qfh, NULL, "POP");
	if (! b)
	{
		SFQ_FAIL(EA_WRITEQFH, "sfq_writeqfh(qfh)");
	}

SFQ_LIB_CHECKPOINT

	if (SFQ_LIB_IS_FAIL())
	{
		sfq_free_ioelm_buff(&ioeb);
	}

SFQ_ENTP_LEAVE

	sfq_close_queue(qo);
	qo = NULL;

	return SFQ_LIB_RC();
}

