/* Copyright (c) 2008, 2009 The Board of Trustees of The Leland Stanford
 * Junior University
 *
 * We are making the OpenFlow specification and associated documentation
 * (Software) available for public use and benefit with the expectation
 * that others will use, modify and enhance the Software and contribute
 * those enhancements back to the community. However, since we would
 * like to make the Software available for broadest use, with as few
 * restrictions as possible permission is hereby granted, free of
 * charge, to any person obtaining a copy of this Software to deal in
 * the Software under the copyrights without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The name and trademarks of copyright holder(s) may NOT be used in
 * advertising or publicity pertaining to the Software or any
 * derivatives without specific, written prior permission.
 */

/* Zoltan:
 * During the move to OpenFlow 1.1 parts of the code was taken from
 * the following repository, with the license below:
 * git://openflow.org/of1.1-spec-test.git (lib/ofp-util.h)
 */
/*
 * Copyright (c) 2008, 2009, 2010 Nicira Networks.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "ofp.h"
#include "ofpbuf.h"
#include "openflow/openflow.h"
#include "openflow/nicira-ext.h"
#include "packets.h"
#include "random.h"
#include "util.h"
//++Modificaciones UAH++//
#include <stdlib.h>
#include "oflib/ofl-messages.h"
#include "oflib/ofl-structs.h"
#include "oflib/ofl-actions.h"
#include "oflib/ofl-print.h"
#include "oflib/ofl.h"
#include "oflib-exp/ofl-exp.h"
#include "oflib-exp/ofl-exp-openflow.h"
#include "oflib/oxm-match.h"
#include <arpa/inet.h>
//++++++//

#define LOG_MODULE VLM_ofp
#include "vlog.h"

//++Modificaciones UAH++//
static void create_ofl_match_UAH(struct flow *flow, struct ofl_match *match);
// static void print_ofp_match_UAH(FILE *stream, struct ofp_match *ofpm, uint8_t *p_oxms, size_t *len);
// static void check_ofp_instructions_UAH(FILE *stream, struct ofp_instruction *ofpia, size_t data_len, size_t *num_in);

//++++++//

/* XXX we should really use consecutive xids to avoid probabilistic
 * failures. */
static inline uint32_t
alloc_xid(void)
{
    return random_uint32();
}

/* Allocates and stores in '*bufferp' a new ofpbuf with a size of
 * 'openflow_len', starting with an OpenFlow header with the given 'type' and
 * an arbitrary transaction id.  Allocated bytes beyond the header, if any, are
 * zeroed.
 *
 * The caller is responsible for freeing '*bufferp' when it is no longer
 * needed.
 *
 * The OpenFlow header length is initially set to 'openflow_len'; if the
 * message is later extended, the length should be updated with
 * update_openflow_length() before sending.
 *
 * Returns the header. */
void *
make_openflow(size_t openflow_len, uint8_t type, struct ofpbuf **bufferp)
{
    *bufferp = ofpbuf_new(openflow_len);
    return put_openflow_xid(openflow_len, type, alloc_xid(), *bufferp);
}

/* Allocates and stores in '*bufferp' a new ofpbuf with a size of
 * 'openflow_len', starting with an OpenFlow header with the given 'type' and
 * transaction id 'xid'.  Allocated bytes beyond the header, if any, are
 * zeroed.
 *
 * The caller is responsible for freeing '*bufferp' when it is no longer
 * needed.
 *
 * The OpenFlow header length is initially set to 'openflow_len'; if the
 * message is later extended, the length should be updated with
 * update_openflow_length() before sending.
 *
 * Returns the header. */
void *
make_openflow_xid(size_t openflow_len, uint8_t type, uint32_t xid,
                  struct ofpbuf **bufferp)
{
    *bufferp = ofpbuf_new(openflow_len);
    return put_openflow_xid(openflow_len, type, xid, *bufferp);
}

/* Appends 'openflow_len' bytes to 'buffer', starting with an OpenFlow header
 * with the given 'type' and an arbitrary transaction id.  Allocated bytes
 * beyond the header, if any, are zeroed.
 *
 * The OpenFlow header length is initially set to 'openflow_len'; if the
 * message is later extended, the length should be updated with
 * update_openflow_length() before sending.
 *
 * Returns the header. */
void *
put_openflow(size_t openflow_len, uint8_t type, struct ofpbuf *buffer)
{
    return put_openflow_xid(openflow_len, type, alloc_xid(), buffer);
}

/* Appends 'openflow_len' bytes to 'buffer', starting with an OpenFlow header
 * with the given 'type' and an transaction id 'xid'.  Allocated bytes beyond
 * the header, if any, are zeroed.
 *
 * The OpenFlow header length is initially set to 'openflow_len'; if the
 * message is later extended, the length should be updated with
 * update_openflow_length() before sending.
 *
 * Returns the header. */
void *
put_openflow_xid(size_t openflow_len, uint8_t type, uint32_t xid,
                 struct ofpbuf *buffer)
{
    struct ofp_header *oh;

    assert(openflow_len >= sizeof *oh);
    assert(openflow_len <= UINT16_MAX);

    oh = ofpbuf_put_uninit(buffer, openflow_len);
    oh->version = OFP_VERSION;
    oh->type = type;
    oh->length = htons(openflow_len);
    oh->xid = xid;
    memset(oh + 1, 0, openflow_len - sizeof *oh);
    return oh;
}

/* Updates the 'length' field of the OpenFlow message in 'buffer' to
 * 'buffer->size'. */
void update_openflow_length(struct ofpbuf *buffer)
{
    struct ofp_header *oh = ofpbuf_at_assert(buffer, 0, sizeof *oh);
    oh->length = htons(buffer->size);
}

/* Updates the 'len' field of the instruction header in 'buffer' to
 * "what it should be"(tm). */
void update_instruction_length(struct ofpbuf *buffer, size_t oia_offset)
{
    struct ofp_header *oh = ofpbuf_at_assert(buffer, 0, sizeof *oh);
    struct ofp_instruction *ih = ofpbuf_at_assert(buffer, oia_offset,
                                                  sizeof *ih);
    ih->len = htons(buffer->size - oia_offset);
}

struct ofpbuf *
make_flow_mod(uint8_t command, uint8_t table_id,
              const struct flow *flow, size_t actions_len)
{
    struct ofp_flow_mod *ofm;
    size_t size;
    struct ofpbuf *out;

    //**Modificaciones UAH**//

    uint8_t *dir_m;
    struct ofl_match *ofl_m = xmalloc(sizeof(struct ofl_match));
    struct flow flow_aux = *flow;
    ofl_structs_match_init(ofl_m);
    create_ofl_match_UAH(&flow_aux, ofl_m); //Se invoca a la función para crear el ofl_match

    size = ROUND_UP(sizeof(struct ofp_flow_mod) - 4 + ofl_m->header.length, 8) + actions_len; //ofl_m->header.length representa el tamaño de los campos de match
    out = ofpbuf_new(size);
    ofm = ofpbuf_put_zeros(out, sizeof *ofm);
    dir_m = (uint8_t *)ofm + sizeof(struct ofp_flow_mod) - 4;                             //Dirección de los oxm_fields del campo match
    ofl_structs_match_pack((struct ofl_match_header *)ofl_m, &(ofm->match), dir_m, NULL); //Se empaqueta la estructura ofl_match en el ofp_flow_mod
    out->size = ROUND_UP(sizeof(struct ofp_flow_mod) - 4 + ofl_m->header.length, 8);

    ofm->header.version = OFP_VERSION;
    ofm->header.type = OFPT_FLOW_MOD;
    ofm->header.length = htons(size);
    ofm->cookie = 0;
    ofm->out_group = OFPG_ANY;
    ofm->out_port = OFPP_ANY;
    ofm->command = command;
    ofm->table_id = table_id;

    ofl_structs_free_match((struct ofl_match_header *)ofl_m, NULL);
    // ofl_structs_free_match((struct ofl_match_header *)ofl_m_aux, NULL);
    /*+++FIN+++*/

    return out;
}

struct ofpbuf *
make_add_flow(const struct flow *flow, uint32_t buffer_id, uint8_t table_id,
              uint16_t idle_timeout, size_t actions_len, uint16_t priority)
{
    //**Modificaciones Boby UAH**//
    struct ofp_flow_mod *ofm;
    struct ofpbuf *out;
    //++++++//
    // Se configura un drop para este flujo
    if (actions_len == 0)
    {
        out = make_flow_mod(OFPFC_ADD, table_id, flow, 0);
    }
    else
    {
        struct ofp_instruction_actions *oia;
        size_t instruction_len = sizeof *oia + actions_len;
        out = make_flow_mod(OFPFC_ADD, table_id, flow, instruction_len);
        /* Use a single apply-actions for now - Jean II */
        oia = ofpbuf_put_zeros(out, sizeof *oia);
        oia->type = htons(OFPIT_APPLY_ACTIONS);
        oia->len = htons(instruction_len);
    }

    ofm = out->data;
    // Se configuran así los timeouts para que las reglas sean permanentes
    ofm->idle_timeout = htons(idle_timeout);
    ofm->hard_timeout = htons(OFP_FLOW_PERMANENT);
    ofm->buffer_id = htonl(buffer_id);
    // Se configura la prioridad de la regla.
    ofm->priority = htons(priority);

    return out;
}

struct ofpbuf *
make_del_flow(const struct flow *flow, uint8_t table_id)
{
    /*Modificaciones Boby UAH*/
    struct ofpbuf *out = make_flow_mod(OFPFC_DELETE, table_id, flow, 0); /*Modificacion Boby UAH*/
    struct ofl_msg_header *ofl_oh;
    /*+++FIN+++*/

    struct ofp_flow_mod *ofm = out->data;
    ofm->out_port = htonl(OFPP_ANY);

    /*Modificaciones Boby UAH*/
    if (!ofl_msg_unpack(out->data, out->size, &ofl_oh, NULL /*xid*/, NULL))
    {
        return out;
    }
    else
    {
        return NULL;
    }
    /*+++FIN+++*/
    // return out;
}

struct ofpbuf *
make_add_simple_flow(const struct flow *flow,
                     uint32_t buffer_id, uint32_t out_port,
                     uint16_t idle_timeout, uint16_t priority)
{
    struct ofpbuf *buffer;
    /*Modificaciones Boby UAH*/
    struct ofl_msg_header *ofl_oh;

    /*+++FIN+++*/
    if (out_port != 0)
    {
        struct ofp_action_output *oao;
        buffer = make_add_flow(flow, buffer_id, 0x00, idle_timeout, sizeof *oao, priority);
        oao = ofpbuf_put_zeros(buffer, sizeof *oao);
        oao->type = htons(OFPAT_OUTPUT);
        oao->len = htons(sizeof *oao);
        oao->port = htonl(out_port);
        oao->max_len = OFPCML_NO_BUFFER; //Para que envíe el paquete completo en el packet_in
        VLOG_WARN(LOG_MODULE, "[MAKE ADD SIMPLE FLOW]: FLOW MOD NORMAL!");
    }
    else
    {
        // Se configura un DROP !!
        buffer = make_add_flow(flow, buffer_id, 0, idle_timeout, 0, priority);

        VLOG_WARN(LOG_MODULE, "[MAKE ADD SIMPLE FLOW]: FLOW MOD DROP!");
    }

    /*Modificaciones Boby UAH*/
    // ip4_dst_aux = flow->nw_dst;
    // ip4_src_aux = flow->nw_src;
    // inet_ntop(AF_INET, &ip4_dst_aux, ip4_dst, INET_ADDRSTRLEN);
    // inet_ntop(AF_INET, &ip4_src_aux, ip4_src, INET_ADDRSTRLEN);
    // // ofm = buffer->data;
    // fprintf(f_ofp, "[MAKE ADD SIMPLE FLOW] Paquete Comando=%d \tin_port = %u \t IP_DST = %s\t IP_SRC = %s \tMatch:", ((struct ofp_flow_mod *)buffer->data)->command, ntohl(flow->in_port), ip4_dst, ip4_src);
    // fprintf(f_ofp, "\n");
    /*+++FIN+++*/

    if (!ofl_msg_unpack(buffer->data, buffer->size, &ofl_oh, NULL /*xid*/, NULL))
    {
        return buffer;
    }
    else
    {
        return NULL;
    }
}

struct ofpbuf *
make_port_desc_request(void)
{

    struct ofp_multipart_request *desc;
    struct ofpbuf *out = ofpbuf_new(sizeof *desc);
    desc = ofpbuf_put_uninit(out, sizeof *desc);
    desc->header.version = OFP_VERSION;
    desc->header.type = OFPT_MULTIPART_REQUEST;
    desc->header.length = htons(sizeof *desc);
    desc->header.xid = alloc_xid();
    desc->type = htons(OFPMP_PORT_DESC);
    desc->flags = 0x0000;
    memset(desc->pad, 0x0, 4);
    return out;
}

struct ofpbuf *
make_packet_out(const struct ofpbuf *packet, uint32_t buffer_id,
                uint32_t in_port,
                const struct ofp_action_header *actions, size_t n_actions)
{
    //Modificaciones UAH//
    size_t actions_len = n_actions * ntohs(actions->len);
    // size_t actions_len = n_actions * sizeof *actions;
    //****//

    struct ofp_packet_out *opo;
    size_t size = sizeof *opo + actions_len + (packet ? packet->size : 0);
    struct ofpbuf *out = ofpbuf_new(size);
    opo = ofpbuf_put_uninit(out, sizeof *opo);
    opo->header.version = OFP_VERSION;
    opo->header.type = OFPT_PACKET_OUT;
    opo->header.length = htons(size);
    opo->header.xid = htonl(0);
    opo->buffer_id = htonl(buffer_id);
    opo->in_port = htonl(in_port);
    opo->actions_len = htons(actions_len);
    ofpbuf_put(out, actions, actions_len);
    if (packet)
    {
        ofpbuf_put(out, packet->data, packet->size);
    }
    return out;
}

struct ofpbuf *
make_unbuffered_packet_out(const struct ofpbuf *packet,
                           uint32_t in_port, uint32_t out_port)
{
    struct ofp_action_output action;
    action.type = htons(OFPAT_OUTPUT);
    action.len = htons(sizeof action);
    action.port = htonl(out_port);
    // VLOG_WARN(LOG_MODULE,"Out port= %u",ntohl(action.port));
    return make_packet_out(packet, UINT32_MAX, in_port,
                           (struct ofp_action_header *)&action, 1);
}

struct ofpbuf *
make_buffered_packet_out(uint32_t buffer_id,
                         uint32_t in_port, uint32_t out_port)
{
    if (out_port != OFPP_ANY)
    {
        struct ofp_action_output action;
        action.type = htons(OFPAT_OUTPUT);
        action.len = htons(sizeof action);
        action.port = htonl(out_port);
        return make_packet_out(NULL, buffer_id, in_port,
                               (struct ofp_action_header *)&action, 1);
    }
    else
    {
        return make_packet_out(NULL, buffer_id, in_port, NULL, 0);
    }
}

/* Creates and returns an OFPT_ECHO_REQUEST message with an empty payload. */
struct ofpbuf *
make_echo_request(void)
{
    struct ofp_header *rq;
    struct ofpbuf *out = ofpbuf_new(sizeof *rq);
    rq = ofpbuf_put_uninit(out, sizeof *rq);
    rq->version = OFP_VERSION;
    rq->type = OFPT_ECHO_REQUEST;
    rq->length = htons(sizeof *rq);
    rq->xid = alloc_xid();
    return out;
}

/* Creates and returns an OFPT_ECHO_REPLY message matching the
 * OFPT_ECHO_REQUEST message in 'rq'. */
struct ofpbuf *
make_echo_reply(const struct ofp_header *rq)
{
    size_t size = ntohs(rq->length);
    struct ofpbuf *out = ofpbuf_new(size);
    struct ofp_header *reply = ofpbuf_put(out, rq, size);
    reply->type = OFPT_ECHO_REPLY;
    return out;
}

static int
check_message_type(uint8_t got_type, uint8_t want_type)
{
    if (got_type != want_type)
    {
        VLOG_WARN(LOG_MODULE, "received bad message type %d (expected %d)",
                  got_type, want_type);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_TYPE);
        ;
    }
    return 0;
}

/* Checks that 'msg' has type 'type' and that it is exactly 'size' bytes long.
 * Returns 0 if the checks pass, otherwise an OpenFlow error code (produced
 * with ofp_mkerr()). */
int check_ofp_message(const struct ofp_header *msg, uint8_t type, size_t size)
{
    size_t got_size;
    int error;

    error = check_message_type(msg->type, type);
    if (error)
    {
        return error;
    }

    got_size = ntohs(msg->length);
    if (got_size != size)
    {
        VLOG_WARN(LOG_MODULE, "received %d message of length %zu (expected %zu)",
                  type, got_size, size);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }

    return 0;
}

/* Checks that 'inst' has type 'type' and that 'inst' is 'size' plus a
 * nonnegative integer multiple of 'array_elt_size' bytes long.  Returns 0 if
 * the checks pass, otherwise an OpenFlow error code (produced with
 * ofp_mkerr()).
 *
 * If 'n_array_elts' is nonnull, then '*n_array_elts' is set to the number of
 * 'array_elt_size' blocks in 'msg' past the first 'min_size' bytes, when
 * successful. */
int check_ofp_instruction_array(const struct ofp_instruction *inst, uint8_t type,
                                size_t min_size, size_t array_elt_size,
                                size_t *n_array_elts)
{
    size_t got_size;

    assert(array_elt_size);

    if (ntohs(inst->type) != type)
    {
        VLOG_WARN(LOG_MODULE, "received bad instruction type %X (expected %X)",
                  ntohs(inst->type), type);
        return ofp_mkerr(OFPET_BAD_INSTRUCTION, OFPBIC_UNSUP_INST);
    }

    got_size = ntohs(inst->len);
    if (got_size < min_size)
    {
        VLOG_WARN(LOG_MODULE, "received %X instruction of length %zu "
                              "(expected at least %zu)",
                  type, got_size, min_size);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }
    if ((got_size - min_size) % array_elt_size)
    {
        VLOG_WARN(LOG_MODULE, "received %X message of bad length %zu: the "
                              "excess over %zu (%zu) is not evenly divisible by %zu "
                              "(remainder is %zu)",
                  type, got_size, min_size, got_size - min_size,
                  array_elt_size, (got_size - min_size) % array_elt_size);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }
    if (n_array_elts)
    {
        *n_array_elts = (got_size - min_size) / array_elt_size;
    }
    return 0;
}

/* Checks that 'msg' has type 'type' and that 'msg' is 'size' plus a
 * nonnegative integer multiple of 'array_elt_size' bytes long.  Returns 0 if
 * the checks pass, otherwise an OpenFlow error code (produced with
 * ofp_mkerr()).
 *
 * If 'n_array_elts' is nonnull, then '*n_array_elts' is set to the number of
 * 'array_elt_size' blocks in 'msg' past the first 'min_size' bytes, when
 * successful.  */
int check_ofp_message_array(const struct ofp_header *msg, uint8_t type,
                            size_t min_size, size_t array_elt_size,
                            size_t *n_array_elts)
{
    size_t got_size;
    int error;

    assert(array_elt_size);

    error = check_message_type(msg->type, type);
    if (error)
    {
        return error;
    }

    got_size = ntohs(msg->length);
    if (got_size < min_size)
    {
        VLOG_WARN(LOG_MODULE, "received %d message of length %zu "
                              "(expected at least %zu)",
                  type, got_size, min_size);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }
    if ((got_size - min_size) % array_elt_size)
    {
        VLOG_WARN(LOG_MODULE,
                  "received %d message of bad length %zu: the "
                  "excess over %zu (%zu) is not evenly divisible by %zu "
                  "(remainder is %zu)",
                  type, got_size, min_size, got_size - min_size,
                  array_elt_size, (got_size - min_size) % array_elt_size);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }
    if (n_array_elts)
    {
        *n_array_elts = (got_size - min_size) / array_elt_size;
    }
    return 0;
}

int check_ofp_packet_out(const struct ofp_header *oh, struct ofpbuf *data,
                         int *n_actionsp, int max_ports)
{
    const struct ofp_packet_out *opo;
    unsigned int actions_len, n_actions;
    size_t extra;
    int error;

    *n_actionsp = 0;
    error = check_ofp_message_array(oh, OFPT_PACKET_OUT,
                                    sizeof *opo, 1, &extra);
    if (error)
    {
        return error;
    }
    opo = (const struct ofp_packet_out *)oh;

    actions_len = ntohs(opo->actions_len);
    if (actions_len > extra)
    {
        VLOG_WARN(LOG_MODULE, "packet-out claims %u bytes of actions "
                              "but message has room for only %zu bytes",
                  actions_len, extra);
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }
    if (actions_len % sizeof(union ofp_action))
    {
        VLOG_WARN(LOG_MODULE, "packet-out claims %u bytes of actions, "
                              "which is not a multiple of %zu",
                  actions_len, sizeof(union ofp_action));
        return ofp_mkerr(OFPET_BAD_REQUEST, OFPBRC_BAD_LEN);
    }

    n_actions = actions_len / sizeof(union ofp_action);
    error = validate_actions((const union ofp_action *)opo->actions,
                             n_actions, max_ports, true);
    if (error)
    {
        return error;
    }

    data->data = (void *)&opo->actions[n_actions];
    data->size = extra - actions_len;
    *n_actionsp = n_actions;
    return 0;
}

/*const struct ofp_flow_stats */
const struct ofp_flow_stats *
flow_stats_first(struct flow_stats_iterator *iter,
                 const struct ofp_multipart_reply *osr)
{
    iter->pos = osr->body;
    iter->end = osr->body + (ntohs(osr->header.length) - offsetof(struct ofp_multipart_reply, body));
    return flow_stats_next(iter);
}

/*const struct ofp_flow_stats */
const struct ofp_flow_stats *
flow_stats_next(struct flow_stats_iterator *iter)
{
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 5);
    ptrdiff_t bytes_left = iter->end - iter->pos;
    const struct ofp_flow_stats *fs;
    size_t length;

    if (bytes_left < sizeof *fs)
    {
        if (bytes_left != 0)
        {
            VLOG_WARN_RL(LOG_MODULE, &rl, "%td leftover bytes in flow stats reply",
                         bytes_left);
        }
        return NULL;
    }

    fs = (const void *)iter->pos;
    length = ntohs(fs->length);
    if (length < sizeof *fs)
    {
        VLOG_WARN_RL(LOG_MODULE, &rl, "flow stats length %zu is shorter than min %zu",
                     length, sizeof *fs);
        return NULL;
    }
    else if (length > bytes_left)
    {
        VLOG_WARN_RL(LOG_MODULE, &rl, "flow stats length %zu but only %td bytes left",
                     length, bytes_left);
        return NULL;
    }
    /* TODO: Change instructions
    else if ((length - sizeof *fs) % sizeof fs->instructions[0]) {
        VLOG_WARN_RL(LOG_MODULE, &rl, "flow stats length %zu has %zu bytes "
                     "left over in final action", length,
                     (length - sizeof *fs) % sizeof fs->instructions[0]);
        return NULL;
    }*/
    iter->pos += length;
    return fs;
}

/* Alignment of ofp_actions. */
#define ACTION_ALIGNMENT 8

static int
check_action_exact_len(const union ofp_action *a, unsigned int len,
                       unsigned int required_len)
{
    if (len != required_len)
    {
        VLOG_DBG(LOG_MODULE, "action %u has invalid length %" PRIu16 " (must be %u)\n",
                 a->type, ntohs(a->header.len), required_len);
        return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_LEN);
    }
    return 0;
}

/* Checks that 'port' is a valid output port for the OFPAT_OUTPUT action, given
 * that the switch will never have more than 'max_ports' ports.  Returns 0 if
 * 'port' is valid, otherwise an ofp_mkerr() return code.*/
static int
check_output_port(uint32_t port, int max_ports, bool table_allowed)
{
    switch (port)
    {
    case OFPP_IN_PORT:
    case OFPP_NORMAL:
    case OFPP_FLOOD:
    case OFPP_ALL:
    case OFPP_CONTROLLER:
    case OFPP_LOCAL:
        return 0;

    case OFPP_TABLE:
        if (table_allowed)
        {
            return 0;
        }
        else
        {
            return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_OUT_PORT);
        }

    default:
        if (port < max_ports)
        {
            return 0;
        }
        VLOG_WARN(LOG_MODULE, "unknown output port %x", port);
        return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_OUT_PORT);
        ;
    }
}

/* Checks that 'action' is a valid OFPAT_ENQUEUE action, given that the switch
 * will never have more than 'max_ports' ports.  Returns 0 if 'port' is valid,
 * otherwise an ofp_mkerr() return code.*/
static int
check_setqueue_action(const union ofp_action *a, unsigned int len)
{
    const struct ofp_action_set_queue *oaq UNUSED;
    int error;

    error = check_action_exact_len(a, len, 8);
    if (error)
    {
        return error;
    }
    /*TODO check if this functions is relevant and finish or
      remove it accordingly */
    /*oaq = (const struct ofp_action_set_queue *) a;*/
    return 0;
}

static int
check_nicira_action(const union ofp_action *a, unsigned int len)
{
    const struct nx_action_header *nah;

    if (len < 16)
    {
        VLOG_DBG(LOG_MODULE, "Nicira vendor action only %u bytes", len);
        return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_LEN);
        ;
    }
    nah = (const struct nx_action_header *)a;

    switch (ntohs(nah->subtype))
    {
    case NXAST_RESUBMIT:
    case NXAST_SET_TUNNEL:
        return check_action_exact_len(a, len, 16);
    default:
        return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_EXPERIMENTER);
    }
}

static int
check_action(const union ofp_action *a, unsigned int len, int max_ports,
             bool is_packet_out)
{
    int error;

    switch (ntohs(a->type))
    {
    case OFPAT_OUTPUT:
    {
        const struct ofp_action_output *oao;
        error = check_action_exact_len(a, len, 16);
        if (error)
        {
            return error;
        }
        oao = (const struct ofp_action_output *)a;
        return check_output_port(ntohl(oao->port), max_ports, is_packet_out);
    }

    case OFPAT_EXPERIMENTER:
        return (a->experimenter.experimenter == htonl(NX_VENDOR_ID)
                    ? check_nicira_action(a, len)
                    : ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_EXPERIMENTER));

    case OFPAT_SET_QUEUE:
        return check_setqueue_action(a, len);

    default:
        VLOG_WARN(LOG_MODULE, "unknown action type %" PRIu16,
                  ntohs(a->type));
        return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_TYPE);
    }
}

int validate_actions(const union ofp_action *actions, size_t n_actions,
                     int max_ports, bool is_packet_out)
{
    const union ofp_action *a;

    for (a = actions; a < &actions[n_actions];)
    {
        unsigned int len = ntohs(a->header.len);
        unsigned int n_slots = len / ACTION_ALIGNMENT;
        unsigned int slots_left = &actions[n_actions] - a;
        int error;

        if (n_slots > slots_left)
        {
            VLOG_DBG(LOG_MODULE,
                     "action requires %u slots but only %u remain",
                     n_slots, slots_left);
            return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_LEN);
        }
        else if (!len)
        {
            VLOG_DBG(LOG_MODULE, "action has invalid length 0");
            return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_LEN);
        }
        else if (len % ACTION_ALIGNMENT)
        {
            VLOG_DBG(LOG_MODULE, "action length %u is not a multiple "
                                 "of %d",
                     len, ACTION_ALIGNMENT);
            return ofp_mkerr(OFPET_BAD_ACTION, OFPBAC_BAD_LEN);
        }

        error = check_action(a, len, max_ports, is_packet_out);
        if (error)
        {
            return error;
        }
        a += n_slots;
    }
    return 0;
}

/* Returns true if 'action' outputs to 'port' (which must be in network byte
 * order), false otherwise. */
bool action_outputs_to_port(const union ofp_action *action, uint32_t port)
{
    switch (ntohs(action->type))
    {
    case OFPAT_OUTPUT:
    {
        const struct ofp_action_output *oao;
        oao = (const struct ofp_action_output *)action;
        return oao->port == port;
    }
    default:
        return false;
    }
}

/* The set of actions must either come from a trusted source or have been
 * previously validated with validate_actions().*/
const union ofp_action *
actions_first(struct actions_iterator *iter,
              const union ofp_action *oa, size_t n_actions)
{
    iter->pos = oa;
    iter->end = oa + n_actions;
    return actions_next(iter);
}

const union ofp_action *
actions_next(struct actions_iterator *iter)
{
    if (iter->pos < iter->end)
    {
        const union ofp_action *a = iter->pos;
        unsigned int len = ntohs(a->header.len);
        iter->pos += len / ACTION_ALIGNMENT;
        return a;
    }
    else
    {
        return NULL;
    }
}
//Modificaciones Boby UAH//
/*Función creada para generar la estructura ofl_match a partir 
  de una estructura flow.
*/
static void create_ofl_match_UAH(struct flow *flow, struct ofl_match *match)
{
    uint32_t ip4_aux;
    char ip4[INET_ADDRSTRLEN];

    // /* Eth type */
    ofl_structs_match_put16(match, OXM_OF_ETH_TYPE, ntohs(flow->dl_type));
    if (flow->dl_type == htons(ETH_TYPE_ARP))
    {
        if (flow->in_port)
        {
            ofl_structs_match_put32(match, OXM_OF_IN_PORT, ntohl(flow->in_port));
        }
        if (flow->nw_dst)
        // if(flow->dl_dst)
        {
            ip4_aux = flow->nw_dst;
            inet_ntop(AF_INET, &ip4_aux, ip4, INET_ADDRSTRLEN);
            VLOG_WARN(LOG_MODULE, "[CREATE OFL MATCH UAH]: ARP destino: %u==%s", flow->nw_dst, ip4);
            // ofl_structs_match_put16(match, OXM_OF_ARP_OP, flow->nw_proto); // ARP_REQ(1) o ARP_REPLY(2)
            ofl_structs_match_put32(match, OXM_OF_ARP_TPA, flow->nw_dst); //IP Target

            // ofl_structs_match_put_eth(match, OXM_OF_ETH_DST, (uint8_t *)value);
            // ofl_structs_match_put32(match, OXM_OF_ARP_SPA, flow->nw_src); //IP Source
        }
        if (flow->dl_dst && !eth_addr_is_zero(flow->dl_dst))
        {
            ofl_structs_match_put_eth(match, OXM_OF_ETH_DST, flow->dl_dst);
        }
    }
    else if (flow->dl_type == htons(ETH_TYPE_IP) && flow->nw_proto == IP_TYPE_TCP)
    {

        ofl_structs_match_put8(match, OXM_OF_IP_PROTO, flow->nw_proto); // TCP
        if (flow->in_port)
        {
            ofl_structs_match_put32(match, OXM_OF_IN_PORT, ntohl(flow->in_port));
        }
        if (flow->nw_dst)
        {
            ofl_structs_match_put32(match, OXM_OF_IPV4_DST, flow->nw_dst); // IP Destino (Controlador)
        }
        if (flow->nw_src)
        {
            ofl_structs_match_put32(match, OXM_OF_IPV4_SRC, flow->nw_src); // IP Origen (Controlador)
        }
    }
}
struct ofpbuf *
modify_local_port_in_band_rules_UAH(const struct flow *flow, uint32_t buffer_id, uint8_t table_id,
                                    uint16_t priority, uint32_t new_local_port)
{
    //**Modificaciones Boby UAH**//
    struct ofp_flow_mod *ofm;
    struct ofpbuf *buffer;

    struct ofp_instruction_actions *oia;
    struct ofp_action_output *oao;
    struct ofl_msg_header *ofl_oh;
    // uint32_t ip4_src_aux, ip4_dst_aux;
    // char ip4_src[INET_ADDRSTRLEN], ip4_dst[INET_ADDRSTRLEN];

    size_t instruction_len = sizeof(struct ofp_instruction_actions) + sizeof(struct ofp_action_output);

    buffer = make_flow_mod(OFPFC_MODIFY, table_id, flow, instruction_len);

    /* Use a single apply-actions for now - Jean II */
    oia = ofpbuf_put_zeros(buffer, sizeof *oia);
    oia->type = htons(OFPIT_APPLY_ACTIONS);
    oia->len = htons(instruction_len);

    ofm = buffer->data;
    ofm->priority = htons(priority);
    ofm->buffer_id = htonl(buffer_id);

    /*Se configura la action output al nuevo puerto*/
    oao = ofpbuf_put_zeros(buffer, sizeof *oao);
    oao->type = htons(OFPAT_OUTPUT);
    oao->len = htons(sizeof(struct ofp_action_output));
    // oao->len = htons(sizeof *oao);
    oao->port = htonl(new_local_port);
    oao->max_len = OFPCML_NO_BUFFER; //Para que envíe el paquete completo en el packet_in
    VLOG_WARN(LOG_MODULE, "[MODIFY LOCAL PORT IN BAND RULES]: Se crea un FLOW MOD para modificar las reglas de in-band");

    if (!ofl_msg_unpack(buffer->data, buffer->size, &ofl_oh, NULL /*xid*/, NULL))
    {
        return buffer;
    }
    else
    {
        return NULL;
    }
    // return buffer;
}
// static void print_ofp_match_UAH(FILE *stream, struct ofp_match *ofpm, uint8_t *p_oxms, size_t* len)
// {
//     struct ofl_match *ofl_match=(struct ofl_match*)malloc(sizeof(struct ofl_match));
//     ofl_err error;
//     ofl_structs_match_init(ofl_match);
//     error=ofl_structs_match_unpack(ofpm, p_oxms, len,(struct ofl_match_header**)&ofl_match, NULL);
//     if(!error)
//     {
//         fprintf(stream, "\n\n##-PRINT OF MATCH UAH-##\n");
//         ofl_structs_match_print(stream, (struct ofl_match_header *)ofl_match, NULL);
//         fprintf(stream, "\n");
//     }else{
//         VLOG_WARN(LOG_MODULE, "[PRINT OFP MATCH UAH]: ¡NO SE HA PODIDO DESEMPAQUETAR EL MATCH!");
//     }

//     ofl_structs_free_match((struct ofl_match_header *)ofl_match, NULL);

//     // free(ofl_match);
// }
// static void check_ofp_instructions_UAH(FILE *stream, struct ofp_instruction *ofpia, size_t data_len, size_t *num_in)
// {
//     ofl_err error;
//     int i;
//     // struct ofl_instruction *ofli_act = (struct ofl_instruction *)malloc(sizeof(struct ofl_instruction_actions));
//     struct ofl_instruction_header **ofli_header;
//     error = ofl_utils_count_ofp_instructions(ofpia, data_len, num_in);
//     if (!error)
//     {
//         fprintf(stream, "\n\n##-CHECK OFP INSTRUCTIONS UAH-##\n");
//         ofli_header = (struct ofl_instruction_header **)malloc((*num_in) * sizeof(struct ofl_instruction_header *));
//         error = ofl_structs_instructions_unpack((struct ofp_instruction *)ofpia, &data_len, ofli_header, NULL);
//         if (!error)
//         {
//             for (i = 0; i < *num_in; i++)
//             {
//                 ofl_structs_instruction_print(stream, ofli_header[i], NULL);
//             }
//         }
//         free(ofli_header);
//     }
//     else
//     {
//         VLOG_WARN(LOG_MODULE, "[CHECK OFP INSTRUCTIONS UAH]: ERROR EN LA COMPROBACIÓN DE LAS INSTRUCCIONES");
//         return;
//     }
// }
//++++++//
