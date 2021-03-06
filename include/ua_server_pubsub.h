/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017-2018 Fraunhofer IOSB (Author: Andreas Ebner)
 */

#ifndef UA_SERVER_PUBSUB_H
#define UA_SERVER_PUBSUB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ua_server.h"

#ifdef UA_ENABLE_PUBSUB

/**
 * .. _pubsub:
 *
 * Publish/Subscribe
 * =================
 *
 * Work in progress!
 * This part will be a new chapter later.
 *
 * TODO: write general PubSub introduction
 *
 * The Publish/Subscribe (PubSub) extension for OPC UA enables fast and efficient
 * 1:m communication. The PubSub extension is protocol agnostic and can be used
 * with broker based protocols like MQTT and AMQP or brokerless implementations like UDP-Multicasting.
 *
 * The PubSub API uses the following scheme:
 *
 * 1. Create a configuration for the needed PubSub element.
 *
 * 2. Call the add[element] function and pass in the configuration.
 *
 * 3. The add[element] function returns the unique nodeId of the internally created element.
 *
 * Take a look on the PubSub Tutorials for mor details about the API usage.::
 *
 *  +-----------+
 *  | UA_Server |
 *  +-----------+
 *   |    |
 *   |    |
 *   |    |
 *   |    |  +----------------------+
 *   |    +--> UA_PubSubConnection  |  UA_Server_addPubSubConnection
 *   |       +----------------------+
 *   |        |    |
 *   |        |    |    +----------------+
 *   |        |    +----> UA_WriterGroup |  UA_PubSubConnection_addWriterGroup
 *   |        |         +----------------+
 *   |        |              |
 *   |        |              |    +------------------+
 *   |        |              +----> UA_DataSetWriter |  UA_WriterGroup_addDataSetWriter  +-+
 *   |        |                   +------------------+                                     |
 *   |        |                                                                            |
 *   |        |         +----------------+                                                 | r
 *   |        +---------> UA_ReaderGroup |                                                 | e
 *   |                  +----------------+                                                 | f
 *   |                                                                                     |
 *   |       +---------------------------+                                                 |
 *   +-------> UA_PubSubPublishedDataSet |  UA_Server_addPublishedDataSet                <-+
 *           +---------------------------+
 *                 |
 *                 |    +-----------------+
 *                 +----> UA_DataSetField |  UA_PublishedDataSet_addDataSetField
 *                      +-----------------+
 * PubSub Information Model Representation
 * -----------
 * The complete PubSub configuration is available inside the information model.
 * The entry point is the node 'PublishSubscribe, located under the Server node.
 * The standard defines for PubSub no new Service set. The configuration can optionally
 * done over methods inside the information model. The information model representation
 * of the current PubSub configuration is generated automatically. This feature
 * can enabled/disable by changing the UA_ENABLE_PUBSUB_INFORMATIONMODEL option.
 *
 * Connections
 * -----------
 * The PubSub connections are the abstraction between the concrete transport protocol
 * and the PubSub functionality. It is possible to create multiple connections with
 * different transport protocols at runtime.
 *
 * Take a look on the PubSub Tutorials for mor details about the API usage.
 */

typedef struct {
    UA_String name;
    UA_Boolean enabled;
    union { /* std: valid types UInt or String */
        UA_UInt32 numeric;
        UA_String string;
    } publisherId;
    UA_String transportProfileUri;
    UA_Variant address;
    size_t connectionPropertiesSize;
    UA_KeyValuePair *connectionProperties;
    UA_Variant connectionTransportSettings;
} UA_PubSubConnectionConfig;

UA_StatusCode
UA_Server_addPubSubConnection(UA_Server *server,
                              const UA_PubSubConnectionConfig *connectionConfig,
                              UA_NodeId *connectionIdentifier);

/* Returns a deep copy of the config */
UA_StatusCode
UA_Server_getPubSubConnectionConfig(UA_Server *server,
                                    const UA_NodeId connection,
                                    UA_PubSubConnectionConfig *config);

/* Remove Connection, identified by the NodeId. Deletion of Connection
 * removes all contained WriterGroups and Writers. */
UA_StatusCode
UA_Server_removePubSubConnection(UA_Server *server, const UA_NodeId connection);

/**
 * PublishedDataSets
 * -----------------
 * The PublishedDataSets (PDS) are containers for the published information. The
 * PDS contain the published variables and meta informations. The metadata is
 * commonly autogenerated or given as constant argument as part of the template
 * functions. The template functions are standard defined and intended for
 * configuration tools. You should normally create a empty PDS and call the
 * functions to add new fields. */

/* The UA_PUBSUB_DATASET_PUBLISHEDITEMS has currently no additional members and
 * thus no dedicated config structure. */

typedef enum {
    UA_PUBSUB_DATASET_PUBLISHEDITEMS,
    UA_PUBSUB_DATASET_PUBLISHEDEVENTS,
    UA_PUBSUB_DATASET_PUBLISHEDITEMS_TEMPLATE,
    UA_PUBSUB_DATASET_PUBLISHEDEVENTS_TEMPLATE,
} UA_PublishedDataSetType;

typedef struct {
    UA_DataSetMetaDataType metaData;
    size_t variablesToAddSize;
    UA_PublishedVariableDataType *variablesToAdd;
} UA_PublishedDataItemsTemplateConfig;

typedef struct {
    UA_NodeId eventNotfier;
    UA_ContentFilter filter;
} UA_PublishedEventConfig;

typedef struct {
    UA_DataSetMetaDataType metaData;
    UA_NodeId eventNotfier;
    size_t selectedFieldsSize;
    UA_SimpleAttributeOperand *selectedFields;
    UA_ContentFilter filter;
} UA_PublishedEventTemplateConfig;

/* Configuration structure for PublishedDataSet */
typedef struct {
    UA_String name;
    UA_PublishedDataSetType publishedDataSetType;
    union {
        /* The UA_PUBSUB_DATASET_PUBLISHEDITEMS has currently no additional members
         * and thus no dedicated config structure.*/
        UA_PublishedDataItemsTemplateConfig itemsTemplate;
        UA_PublishedEventConfig event;
        UA_PublishedEventTemplateConfig eventTemplate;
    } config;
} UA_PublishedDataSetConfig;

void
UA_PublishedDataSetConfig_deleteMembers(UA_PublishedDataSetConfig *pdsConfig);

typedef struct {
    UA_StatusCode addResult;
    size_t fieldAddResultsSize;
    UA_StatusCode *fieldAddResults;
    UA_ConfigurationVersionDataType configurationVersion;
} UA_AddPublishedDataSetResult;

UA_AddPublishedDataSetResult
UA_Server_addPublishedDataSet(UA_Server *server,
                              const UA_PublishedDataSetConfig *publishedDataSetConfig,
                              UA_NodeId *pdsIdentifier);

/* Returns a deep copy of the config */
UA_StatusCode
UA_Server_getPublishedDataSetConfig(UA_Server *server, const UA_NodeId pds,
                                    UA_PublishedDataSetConfig *config);

/* Remove PublishedDataSet, identified by the NodeId. Deletion of PDS removes
 * all contained and linked PDS Fields. Connected WriterGroups will be also
 * removed. */
UA_StatusCode
UA_Server_removePublishedDataSet(UA_Server *server, const UA_NodeId pds);

/**
 * DataSetFields
 * -------------
 * The description of published variables is named DataSetField. Each
 * DataSetField contains the selection of one information model node. The
 * DataSetField has additional parameters for the publishing, sampling and error
 * handling process. */

typedef struct{
    UA_ConfigurationVersionDataType configurationVersion;
    UA_String fieldNameAlias;
    UA_Boolean promotedField;
    UA_PublishedVariableDataType publishParameters;
} UA_DataSetVariableConfig;

typedef enum {
    UA_PUBSUB_DATASETFIELD_VARIABLE,
    UA_PUBSUB_DATASETFIELD_EVENT
} UA_DataSetFieldType;

typedef struct {
    UA_DataSetFieldType dataSetFieldType;
    union {
        UA_DataSetVariableConfig variable;
        //events need other config later
    } field;
} UA_DataSetFieldConfig;
    
void
UA_DataSetFieldConfig_deleteMembers(UA_DataSetFieldConfig *dataSetFieldConfig);

typedef struct {
    UA_StatusCode result;
    UA_ConfigurationVersionDataType configurationVersion;
} UA_DataSetFieldResult;

UA_DataSetFieldResult
UA_Server_addDataSetField(UA_Server *server,
                          const UA_NodeId publishedDataSet,
                          const UA_DataSetFieldConfig *fieldConfig,
                          UA_NodeId *fieldIdentifier);

/* Returns a deep copy of the config */
UA_StatusCode
UA_Server_getDataSetFieldConfig(UA_Server *server, const UA_NodeId dsf,
                                UA_DataSetFieldConfig *config);

UA_DataSetFieldResult
UA_Server_removeDataSetField(UA_Server *server, const UA_NodeId dsf);

/**
 * WriterGroup
 * -----------
 * All WriterGroups are created within a PubSubConnection and automatically
 * deleted if the connection is removed. The WriterGroup is primary used as
 * container for :ref:`dsw` and network message settings. The WriterGroup can be
 * imagined as producer of the network messages. The creation of network
 * messages is controlled by parameters like the publish interval, which is e.g.
 * contained in the WriterGroup. */

typedef enum {
    UA_PUBSUB_ENCODING_BINARY,
    UA_PUBSUB_ENCODING_JSON,
    UA_PUBSUB_ENCODING_UADP
} UA_PubSubEncodingType;

typedef struct {
    UA_String name;
    UA_Boolean enabled;
    UA_UInt16 writerGroupId;
    UA_Duration publishingInterval;
    UA_Double keepAliveTime;
    UA_Byte priority;
    UA_MessageSecurityMode securityMode;
    UA_ExtensionObject transportSettings;
    UA_ExtensionObject messageSettings;
    size_t groupPropertiesSize;
    UA_KeyValuePair *groupProperties;
    UA_PubSubEncodingType encodingMimeType;

    /* non std. config parameter. maximum count of embedded DataSetMessage in
     * one NetworkMessage */
    UA_UInt16 maxEncapsulatedDataSetMessageCount;
} UA_WriterGroupConfig;

void
UA_WriterGroupConfig_deleteMembers(UA_WriterGroupConfig *writerGroupConfig);

/* Add a new WriterGroup to an existing Connection */
UA_StatusCode
UA_Server_addWriterGroup(UA_Server *server, const UA_NodeId connection,
                         const UA_WriterGroupConfig *writerGroupConfig,
                         UA_NodeId *writerGroupIdentifier);

/* Returns a deep copy of the config */
UA_StatusCode
UA_Server_getWriterGroupConfig(UA_Server *server, const UA_NodeId writerGroup,
                               UA_WriterGroupConfig *config);

UA_StatusCode
UA_Server_updateWriterGroupConfig(UA_Server *server, UA_NodeId writerGroupIdentifier,
                                  const UA_WriterGroupConfig *config);

UA_StatusCode
UA_Server_removeWriterGroup(UA_Server *server, const UA_NodeId writerGroup);

/**
 * .. _dsw:
 *
 * DataSetWriter
 * -------------
 * The DataSetWriters are the glue between the WriterGroups and the
 * PublishedDataSets. The DataSetWriter contain configuration parameters and
 * flags which influence the creation of DataSet messages. These messages are
 * encapsulated inside the network message. The DataSetWriter must be linked
 * with an existing PublishedDataSet and be contained within a WriterGroup. */

typedef struct {
    UA_String name;
    UA_UInt16 dataSetWriterId;
    UA_DataSetFieldContentMask dataSetFieldContentMask;
    UA_UInt32 keyFrameCount;
    UA_ExtensionObject messageSettings;
    UA_String dataSetName;
    size_t dataSetWriterPropertiesSize;
    UA_KeyValuePair *dataSetWriterProperties;
} UA_DataSetWriterConfig;

void
UA_DataSetWriterConfig_deleteMembers(UA_DataSetWriterConfig *pdsConfig);

/* Add a new DataSetWriter to a existing WriterGroup. The DataSetWriter must be
 * coupled with a PublishedDataSet on creation.
 *
 * Part 14, 7.1.5.2.1 defines: The link between the PublishedDataSet and
 * DataSetWriter shall be created when an instance of the DataSetWriterType is
 * created. */
UA_StatusCode
UA_Server_addDataSetWriter(UA_Server *server,
                           const UA_NodeId writerGroup, const UA_NodeId dataSet,
                           const UA_DataSetWriterConfig *dataSetWriterConfig,
                           UA_NodeId *writerIdentifier);

/* Returns a deep copy of the config */
UA_StatusCode
UA_Server_getDataSetWriterConfig(UA_Server *server, const UA_NodeId dsw,
                                 UA_DataSetWriterConfig *config);

UA_StatusCode
UA_Server_removeDataSetWriter(UA_Server *server, const UA_NodeId dsw);

#endif /* UA_ENABLE_PUBSUB */
    
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UA_SERVER_PUBSUB_H */
