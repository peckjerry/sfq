#include "sfq-lib.h"

int sfq_push(const char* querootdir, const char* quename, const struct sfq_value* val)
{
LIBFUNC_INITIALIZE

	struct sfq_queue_object* qo = NULL;
	struct sfq_process_info* procs = NULL;

	int i = 0;
	bool b = false;

	int slotno = -1;
	size_t iosize = 0;
	size_t eh_size = 0;
	off_t elm_pos = 0;

	off_t IfPush_next_elmpos = 0;
	off_t IfPush_elm_end_pos = 0;

	struct sfq_file_header qfh;
	struct sfq_e_header prev_eh;
	struct sfq_ioelm_buff ioeb;

/* initialize */
	eh_size = sizeof(struct sfq_e_header);

	bzero(&qfh, sizeof(qfh));
	bzero(&prev_eh, sizeof(prev_eh));
	bzero(&ioeb, sizeof(ioeb));

/* check argument */
	if (! val)
	{
		FIRE(SFQ_RC_EA_FUNCARG, "val is null");
	}

/* open queue-file */
	qo = sfq_open_queue(querootdir, quename, "rb+");
	if (! qo)
	{
		FIRE(SFQ_RC_EA_OPENFILE, "open_locked_file");
	}

/* read file-header */
	b = sfq_readqfh(qo->fp, &qfh, &procs);
	if (! b)
	{
		FIRE(SFQ_RC_EA_READQFH, "sfq_readqfh");
	}

	qfh.last_qhd2 = qfh.last_qhd1;
	qfh.last_qhd1 = qfh.qh.dval;

/* create element-header */
	b = sfq_copy_val2ioeb(val, &ioeb);
	if (! b)
	{
		FIRE(SFQ_RC_EA_COPYVALUE, "sfq_copy_val2ioeb");
	}

/* check payload-size limit */
	if (qfh.qh.sval.payloadsize_limit)
	{
		if (val->payload_size > qfh.qh.sval.payloadsize_limit)
		{
			FIRE(SFQ_RC_EA_OVERLIMIT, "val->payload_size > qfh.qh.sval.payloadsize_limit");
		}
	}

	/* 最終 id を加算 */
	qfh.qh.dval.elm_lastid++;

	/* id, pushtime はここで生成する */
	ioeb.eh.id = qfh.qh.dval.elm_lastid;
	ioeb.eh.pushtime = qo->opentime;

/* set cursor to pushable-pos */
	if (qfh.qh.dval.elm_next_shift_pos == qfh.qh.dval.elm_new_push_pos)
	{
	/* shift 位置 == push 位置 の場合 ... 全て shift している */

		if (qfh.qh.dval.elm_num != 0)
		{
			FIRE(SFQ_RC_EA_ASSERT, "qfh.qh.dval.elm_num != 0");
		}

		// GO NEXT
	}
	else if (qfh.qh.dval.elm_next_shift_pos < qfh.qh.dval.elm_new_push_pos)
	{
	/* shift 位置 < push 位置 の場合 ... 通常 */

		// GO NEXT
	}
	else
	{
	/* push 位置 < shift 位置 の場合 ... 循環済 */

		if (qfh.qh.dval.elm_num == 0)
		{
			FIRE(SFQ_RC_EA_ASSERT, "qfh.qh.dval.elm_num == 0");
		}

		IfPush_next_elmpos = qfh.qh.dval.elm_new_push_pos + ioeb.eh.elmsize_;
		IfPush_elm_end_pos = IfPush_next_elmpos - 1;

		if (IfPush_elm_end_pos < (qfh.qh.dval.elm_next_shift_pos - 1))
		{
		/* 追加しても次回の shift 位置を越えない */

/*
本来はこの条件判定は IfPush_elm_end_pos < qfh.qh.dval.elm_next_shift_pos で良いのだが
そーすると "次回 push 位置" == "次回 shift 位置" になってしまい、その後が
面倒なので IfPush_elm_end_pos と qfh.qh.dval.elm_next_shift_pos の間に 1 byte 空ける
*/

			// GO NEXT
		}
		else
		{
			FIRE(SFQ_RC_NO_SPACE, "no space/shift");
		}
	}

	IfPush_next_elmpos = qfh.qh.dval.elm_new_push_pos + ioeb.eh.elmsize_;
	IfPush_elm_end_pos = IfPush_next_elmpos - 1;

	if (IfPush_elm_end_pos <= qfh.qh.sval.elmseg_end_pos)
	{
	/* 領域限界を超えない ... 通常 */

		/* push 位置へ移動 */
		elm_pos = qfh.qh.dval.elm_new_push_pos;

		/* 要素ヘッダを書き換え */
		ioeb.eh.prev_elmpos = qfh.qh.dval.elm_last_push_pos;

		/* 属性ヘッダを書き換え */
		qfh.qh.dval.elm_last_push_pos = qfh.qh.dval.elm_new_push_pos;
		qfh.qh.dval.elm_new_push_pos = IfPush_next_elmpos;
	}
	else
	{
	/* 領域限界を超える */

		IfPush_next_elmpos = qfh.qh.sval.elmseg_start_pos + ioeb.eh.elmsize_;
		IfPush_elm_end_pos = IfPush_next_elmpos - 1;

		if (IfPush_elm_end_pos < (qfh.qh.dval.elm_next_shift_pos - 1))
		{
		/* 先頭に領域がある --> 循環 */

/*
本来はこの条件判定は IfPush_elm_end_pos < qfh.qh.dval.elm_next_shift_pos で良いのだが
そーすると "次回 push 位置" == "次回 shift 位置" になってしまい、その後が
面倒なので IfPush_elm_end_pos と qfh.qh.dval.elm_next_shift_pos の間に 1 byte 空ける
*/

			/* push 位置へ移動 */
			elm_pos = qfh.qh.sval.elmseg_start_pos;

			/* 要素ヘッダを書き換え */
			ioeb.eh.prev_elmpos = qfh.qh.dval.elm_last_push_pos;

			/* 属性ヘッダを書き換え */
			qfh.qh.dval.elm_last_push_pos = qfh.qh.sval.elmseg_start_pos;
			qfh.qh.dval.elm_new_push_pos = IfPush_next_elmpos;

			if (qfh.qh.dval.elm_num == 0)
			{
			/* 次回の shift 位置も書き換え */

				qfh.qh.dval.elm_next_shift_pos = qfh.qh.sval.elmseg_start_pos;
			}
		}
		else
		{
			FIRE(SFQ_RC_NO_SPACE, "no space/segment");
		}
	}

#ifdef SFQ_DEBUG_BUILD
	sfq_print_e_header(&ioeb.eh);
#endif

/* write element */
	b = sfq_writeelm(qo->fp, elm_pos, &ioeb);
	if (! b)
	{
		FIRE(SFQ_RC_EA_RWELEMENT, "sfq_writeelm");
	}

	if (qfh.qh.dval.elm_num == 0)
	{
	/* 初回 */

		/* 次回の shift 位置を先頭位置に移動 */
		qfh.qh.dval.elm_next_shift_pos = qfh.qh.sval.elmseg_start_pos;
	}

	if (ioeb.eh.prev_elmpos)
	{
/* update prev-element */
		/* next_elmpos を書き換え、リンクをつなげる */

		b = sfq_seek_set_and_read(qo->fp, ioeb.eh.prev_elmpos, &prev_eh, eh_size);
		if (! b)
		{
			FIRE(SFQ_RC_EA_SEEKSETIO, "sfq_seek_set_and_read(prev_eh)");
		}

		prev_eh.next_elmpos = elm_pos;

		b = sfq_seek_set_and_write(qo->fp, ioeb.eh.prev_elmpos, &prev_eh, eh_size);
		if (! b)
		{
			FIRE(SFQ_RC_EA_SEEKSETIO, "sfq_seek_set_and_write(prev_eh)");
		}
	}

/* update queue file header */
	/* 要素数を加算 */
	qfh.qh.dval.elm_num++;

	/* デバッグ用 */
	strcpy(qfh.qh.dval.lastoper, "PSH");
	qfh.qh.dval.update_num++;
	qfh.qh.dval.updatetime = qo->opentime;

/* overwrite header */
	b = sfq_seek_set_and_write(qo->fp, 0, &qfh, sizeof(qfh));
	if (! b)
	{
		FIRE(SFQ_RC_EA_SEEKSETIO, "sfq_seek_set_and_write(qfh)");
	}

#ifdef SFQ_DEBUG_BUILD
	sfq_print_q_header(&qfh.qh);
#endif

	if (procs)
	{
/* boot executer */
		for (i=0; i<qfh.qh.sval.max_proc_num; i++)
		{
			if (procs[i].procstatus)
			{
				continue;
			}

			procs[i].ppid = getpid();
			procs[i].procstatus = SFQ_PIS_WAITFOR;
			procs[i].updtime = qo->opentime;

			slotno = i;

			break;
		}

		if (slotno != -1)
		{
			size_t procs_size = sizeof(struct sfq_process_info) * qfh.qh.sval.max_proc_num;

/* overwrite process-table */
			iosize = fwrite(procs, procs_size, 1, qo->fp);
			if (iosize != 1)
			{
				FIRE(SFQ_RC_ES_FILEIO, "fwrite(procs)");
			}

#ifdef SFQ_DEBUG_BUILD
			sfq_print_procs(procs, qfh.qh.sval.max_proc_num);
#endif
		}
	}

LIBFUNC_COMMIT

	sfq_close_queue(qo);
	qo = NULL;

	free(procs);
	procs = NULL;

	if (slotno != -1)
	{
		sfq_go_exec(querootdir, quename, slotno);
	}

LIBFUNC_FINALIZE

	return LIBFUNC_RC();
}
