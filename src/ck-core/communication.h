/***************************************************************************
 * RCS INFORMATION:
 *
 *	$RCSfile$
 *	$Author$	$Locker$		$State$
 *	$Revision$	$Date$
 *
 ***************************************************************************
 * DESCRIPTION:
 *
 ***************************************************************************
 * REVISION HISTORY:
 *
 * $Log$
 * Revision 2.0  1995-06-02 17:27:40  brunner
 * Reorganized directory structure
 *
 * Revision 1.10  1995/04/24  20:06:06  sanjeev
 * *** empty log message ***
 *
 * Revision 1.9  1995/04/24  19:52:44  sanjeev
 * Changed CkAsyncSend to Cmi
 * Changed CkAsyncSend to CmiSyncSend in CkSend()
 *
 * Revision 1.8  1995/04/23  21:02:26  sanjeev
 * Removed Core...
 *
 * Revision 1.7  1995/04/13  20:53:46  sanjeev
 * Changed Mc to Cmi
 *
 * Revision 1.6  1995/03/25  18:25:47  sanjeev
 * *** empty log message ***
 *
 * Revision 1.5  1995/03/24  16:42:38  sanjeev
 * *** empty log message ***
 *
 * Revision 1.4  1995/03/17  23:37:42  sanjeev
 * changes for better message format
 *
 * Revision 1.3  1995/03/12  17:09:37  sanjeev
 * changes for new msg macros
 *
 * Revision 1.2  1994/11/11  05:31:23  brunner
 * Removed ident added by accident with RCS header
 *
 * Revision 1.1  1994/11/07  15:39:45  brunner
 * Initial revision
 *
 ***************************************************************************/
#define UNPACK(envelope) if (GetEnv_isPACKED(envelope) == PACKED) \
{ \
        void *unpackedUsrMsg; \
        void *usrMsg = USER_MSG_PTR(envelope); \
        (*(MsgToStructTable[GetEnv_packid(envelope)].unpack)) \
                (usrMsg, &unpackedUsrMsg); \
        if (usrMsg != unpackedUsrMsg) \
        /* else unpacked in place */ \
        { \
                int temp_i; \
                int temp_size; \
                char *temp1, *temp2; \
                /* copy envelope */ \
                temp1 = (char *) envelope; \
                temp2 = (char *) ENVELOPE_UPTR(unpackedUsrMsg); \
                temp_size = (char *) usrMsg - temp1; \
                for (temp_i = 0; temp_i<temp_size; temp_i++) \
                        *temp2++ = *temp1++; \
                CmiFree(envelope); \
                envelope = ENVELOPE_UPTR(unpackedUsrMsg); \
        } \
        SetEnv_isPACKED(envelope, UNPACKED); \
}


#define PACK(env) 	if (GetEnv_isPACKED(env) == UNPACKED) \
        /* needs packing and not already packed */ \
        { \
		int size; \
        	char *usermsg, *packedmsg; \
                /* make it +ve to connote a packed msg */ \
                SetEnv_isPACKED(env, PACKED); \
                usermsg = USER_MSG_PTR(env); \
                (*(MsgToStructTable[GetEnv_packid(env)].pack)) \
                        (usermsg, &packedmsg, &size); \
                if (usermsg != packedmsg) \
                        env = ENVELOPE_UPTR(packedmsg); \
        }\



#define CkCheck_and_Send(env,Entry) { \
	if ( GetEnv_destPE(env) == CmiMyPe()) { \
	        CmiSetHandler(env,CallProcessMsg_Index) ; \
                CsdEnqueue(env); \
	} \
        else \
		CkSend(GetEnv_destPE(env), env); \
	}

/* NOTE : The CmiSize call below is required because the 
   msg size might change after packing */

#define CkSend(pe,env) \
{ \
	LdbFillBlock(env); \
	PACK(env); \
	CmiSetHandler(env,HANDLE_INCOMING_MSG_Index); \
	CmiSyncSend(pe,CmiSize(env),env); \
	CmiFree(env) ; \
}


#define CkCheck_and_Broadcast(env,Entry) { \
        LdbFillBlock(env); PACK(env); \
        CmiSetHandler(env,HANDLE_INCOMING_MSG_Index); \
	CmiSyncBroadcast(CmiSize(env),env); \
	CmiFree(env) ; \
        }

#define CkCheck_and_BroadcastNoFree(env,Entry) { \
        LdbFillBlock(env); PACK(env); \
        CmiSetHandler(env,HANDLE_INCOMING_MSG_Index); \
	CmiSyncBroadcast(CmiSize(env),env); UNPACK(env);  \
        }

#define CkCheck_and_BroadcastAll(env,Entry) { \
        LdbFillBlock(env); PACK(env); \
        CmiSetHandler(env,HANDLE_INCOMING_MSG_Index); \
	CmiSyncBroadcastAllAndFree(CmiSize(env),env);\
        }
