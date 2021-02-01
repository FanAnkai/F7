/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTClient.h"

extern unsigned int os_tick;

void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessgage) {
    md->topicName = aTopicName;
    md->message = aMessgage;
}


int getNextPacketId(Client *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}


int sendPacket(Client* c, int length, unsigned long timeout_ms)
{
    int rc = FAILURE, sent = 0;
    unsigned long count = 0;

    while (sent < length && count < c->command_count)
    {
        rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length-sent, timeout_ms);
        if (rc < 0) {  // there was an error writing the data
            break;
        }
        else if(rc == 0) {
            return SEND_TIMOUT;
        }
        sent += rc;
        count++;
    }
    if (sent == length)
    {
        countdown(&c->ping_timer, c->keepAliveInterval); // record the fact that we have successfully sent the packet,keepAliveInterval is 5 s
        rc = SUCCESS;
    } else {
        rc = FAILURE;
    }

    return rc;
}


void MQTTClient(Client* c, Network* network, unsigned int command_timeout_ms, unsigned char* buf, type_size_t buf_size, unsigned char* readbuf, type_size_t readbuf_size)
{
    int i;
    c->ipstack = network;
    
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = 0;
    c->command_timeout_ms = command_timeout_ms;
    c->command_count = MAX_COMMAND_COUNT;
    c->buf = buf;
    c->buf_size = buf_size;
    c->readbuf = readbuf;
    c->readbuf_size = readbuf_size;
    c->isconnected = 0;
    c->ping_outstanding = 0;

    c->lastreceive_tp = 0;

    c->defaultMessageHandler = NULL;
    InitTimer(&c->ping_timer);
}


int decodePacket(Client* c, int* value, unsigned long timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }
        rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
        if (rc != 1)
            goto exit;
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}

int readPacket_30ms(Client* c, unsigned long timeout_ms)
{
    int rc = FAILURE;
    MQTTHeader header = {0};
    int rem_len = 0;
    int readlen = 0;
    int needread = 0;
    int totalread = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (c->ipstack->mqttread(c->ipstack, c->readbuf, 1, 30) != 1) {
        goto exit;
    }

    totalread = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, timeout_ms);
    totalread += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    if (totalread+rem_len >= c->readbuf_size) {
        ql_log_err( ERR_EVENT_INTERFACE , "rps:overflow %d", totalread+rem_len);
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    needread = rem_len;
    while (needread > 0) { 
	    readlen = c->ipstack->mqttread(c->ipstack, c->readbuf + totalread, needread, timeout_ms);
	    if (readlen < 0) {
		    goto exit;
	    }   
	    needread -= readlen;
	    totalread += readlen;
    }

    /*
    if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, timeout_ms) != rem_len)) {
        goto exit;
    }
    */

    header.byte = c->readbuf[0];
    rc = header.bits.type;

exit:
    return rc;
}


int readPacket(Client* c, unsigned long timeout_ms)
{
    int rc = FAILURE;
    MQTTHeader header = {0};
    int rem_len = 0;
    int readlen = 0;
    int needread = 0;
    int totalread = 0;

    /* 1. read the header byte.  This has the packet type in it */
    if (c->ipstack->mqttread(c->ipstack, c->readbuf, 1, timeout_ms) != 1) {
        goto exit;
    }

    totalread = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, timeout_ms);
    totalread += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    if (totalread+rem_len >= c->readbuf_size) {
	    ql_log_err( ERR_EVENT_INTERFACE, "rp:overflow %d", totalread+rem_len);
	    goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    needread = rem_len;
    while (needread > 0) { 
	    readlen = c->ipstack->mqttread(c->ipstack, c->readbuf + totalread, needread, timeout_ms);
	    if (readlen < 0) {
		    goto exit;
	    }   
	    needread -= readlen;
	    totalread += readlen;
    }

    /*
    if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, timeout_ms) != rem_len)) {
        goto exit;
    }
    */

    header.byte = c->readbuf[0];
    rc = header.bits.type;

exit:
    return rc;
}


// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;
    
    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };
    
    return (curn == curn_end) && (*curf == '\0');
}


int deliverMessage(Client* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md);
                rc = SUCCESS;
            }
        }
    }
    
    if (rc == FAILURE && c->defaultMessageHandler != NULL) 
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }   
    
    return rc;
}


static int ql_mq_keepalive(Client* c)
{
    int rc = FAILURE;

    if (c->keepAliveInterval == 0)
    {
        rc = SUCCESS;
        goto exit;
    }
    if (expired(&c->ping_timer))
    {
        c->ping_outstanding = 0;
        if (!c->ping_outstanding)
        {
            int len = MQTTSerialize_pingreq(c->buf, c->buf_size);
            if (len > 0) {
            	rc = sendPacket(c, len, c->command_timeout_ms);
            	if (rc == SUCCESS) {
            		c->ping_outstanding = 1;
            	} else {
            	}
            }
        }
    }

exit:
    return rc;
}

void  cloud_ota_cmd_msg_id_set(type_u16 mq_msg_id);
void  cloud_ota_cmd_msg_id_cmp(type_u16 mq_msg_id);

int cycle(Client* c, unsigned long timeout_ms)
{
    unsigned short mypacketid;
    unsigned char dup, type;
    // receive packet,the timeout of first byte is fixed 30 ms
    int packet_type = readPacket_30ms(c, timeout_ms);

    int len = 0, rc = SUCCESS;

    switch (packet_type)
    {
        case PUBACK:
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
            {
                rc = FAILURE;
            }
            else
            {
                cloud_ota_cmd_msg_id_cmp(mypacketid);
            }
            break;
        case CONNACK:
        case SUBACK:
        case UNSUBACK:
            break;
        case PUBLISH:
        {
            MQTTString topicName;
            MQTTMessage msg;
            if (MQTTDeserialize_publish((unsigned char*)&msg.dup, (int*)&msg.qos, (unsigned char*)&msg.retained, (unsigned short*)&msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = FAILURE;
                else {
                    rc = sendPacket(c, len, c->command_timeout_ms);
                }
                if (rc == FAILURE)
                    goto exit; // there was a problem
            }
            break;
        }
        case PUBREC:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
                rc = FAILURE;
            else if ((rc = sendPacket(c, len, c->command_timeout_ms)) != SUCCESS) // send the PUBREL packet
                rc = FAILURE; // there was a problem
            if (rc == FAILURE)
                goto exit; // there was a problem
            break;
        }
        case PUBCOMP:
            break;
        case PINGRESP:
            c->ping_outstanding = 0;
            break;
    }

    ql_mq_keepalive(c);

    if (packet_type > 0) 
    {
	    c->lastreceive_tp = os_tick;
    }

exit:
    if (rc == SUCCESS)
        rc = packet_type;

    return rc;
}


int MQTTYield(Client* c, unsigned long timeout_ms)
{
    int rc = SUCCESS;
    int count = 0;

    while (count < c->command_count) {
    	cycle(c, timeout_ms);
    	count++;
    }

    return rc;
}


// only used in single-threaded mode where one command at a time is in process
/*
int waitfor(Client* c, int packet_type, Timer* timer)
{
    int rc = FAILURE;
    
    do
    {
        if (expired(timer)) 
            break; // we timed out
    }
    while ((rc = cycle(c, timer)) != packet_type);  
    
    return rc;
}
*/

int cycle_ack(Client* c, unsigned long timeout_ms)
{
    unsigned short packet_type = readPacket(c, timeout_ms);

    return packet_type;
}

int waitforack(Client* c, int packet_type)
{
    int rc = FAILURE;
    int count = 0;

    do {
    	rc = cycle_ack(c, c->command_timeout_ms);
    	count++;
        if(rc == PINGRESP)
        {
            c->ping_outstanding = 0;
        }
    } while (count < c->command_count && packet_type != rc);

    return rc;
}

int MQTTConnect(Client* c, MQTTPacket_connectData* options)
{
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;

    if (c->isconnected) {// don't send connect packet again if we are already connected
    	goto exit;
    }

    if (options == NULL) {
        options = &default_options; // set default options if none were supplied
    }
    
    c->keepAliveInterval = options->keepAliveInterval;
    countdown(&c->ping_timer, c->keepAliveInterval);

    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0) {
    	goto exit;
    }

    if ((rc = sendPacket(c, len, c->command_timeout_ms)) != SUCCESS) {  // send the connect packet
    	goto exit; // there was a problem
    }
    

    // this will be a blocking call, wait for the connack
    if (waitforack(c, CONNACK) == CONNACK)
    {
        unsigned char connack_rc = 255;
        char sessionPresent = 0;
        if (MQTTDeserialize_connack((unsigned char*)&sessionPresent, &connack_rc, c->readbuf, c->readbuf_size) == 1) {
            rc = connack_rc;
        } else {
            rc = FAILURE;
        }
    } else {
        rc = FAILURE;
    }

exit:
    if (rc == SUCCESS)
        c->isconnected = 1;
    return rc;
}

int MQTTSubscribe(Client* c, const char* topicFilter, int ret, messageHandler messageHandler, enum QoS qos)
{ 
    int rc = FAILURE;  
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    if (!c->isconnected) {
        goto exit;
    }
    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0) {
    	goto exit;
    }
    if ((rc = sendPacket(c, len, c->command_timeout_ms)) != SUCCESS) {// send the subscribe packet

    	goto exit;             // there was a problem
    }

    if (waitforack(c, SUBACK) == SUBACK)      // wait for suback
    {
        int count = 0, grantedQoS = -1;
        unsigned short mypacketid;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, c->readbuf, c->readbuf_size) == 1)
            rc = grantedQoS; // 0, 1, 2 or 0x80 
        if (rc != 0x80)
        {
            int i;
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == 0)
                {
                    c->messageHandlers[i].topicFilter = topicFilter;
                    c->messageHandlers[i].fp = messageHandler;
                    rc = 0;
                    break;
                }
            }
        }
    } else {
        rc = FAILURE;
    }

exit:
    return rc;
}

int MQTTPublish(Client* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;
    
    if (!c->isconnected)
        goto exit;

    //if (message->qos == QOS1 || message->qos == QOS2)
    message->id = getNextPacketId(c);

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id, 
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, c->command_timeout_ms)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem
    

    if (message->qos == QOS1)
    {
        cloud_ota_cmd_msg_id_set(message->id);

        if (waitforack(c, PUBACK) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
            {
                rc = FAILURE;
            }
            else
            {
                cloud_ota_cmd_msg_id_cmp(mypacketid);
            }
        }
        else
            rc = FAILURE;
    }
    else if (message->qos == QOS2)
    {
        if (waitforack(c, PUBCOMP) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }

exit:
    return rc;
}


int MQTTDisconnect(Client* c)
{  
    int rc = FAILURE;
    int len = MQTTSerialize_disconnect(c->buf, c->buf_size);

    if (len > 0)
        rc = sendPacket(c, len, c->command_timeout_ms);            // send the disconnect packet

    if (rc == SUCCESS) {
    	c->isconnected = 0;
    }

    return rc;
}
