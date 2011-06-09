/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/


#include "common/Messages.hh"
#include "transport/Publication.hh"
#include "transport/TopicManager.hh"

using namespace gazebo;
using namespace transport;

////////////////////////////////////////////////////////////////////////////////
// Constructor
TopicManager::TopicManager()
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
TopicManager::~TopicManager()
{
}

////////////////////////////////////////////////////////////////////////////////
// Init the topic Manager
void TopicManager::Init()
{
  this->advertisedTopics.clear();
  this->subscribed_topics.clear();
}

void TopicManager::Fini()
{
  this->advertisedTopics.clear();
  this->subscribed_topics.clear();
}


////////////////////////////////////////////////////////////////////////////////
/// Send a message
void TopicManager::Publish( const std::string &topic, 
                            google::protobuf::Message &message,
                            const boost::function<void()> &cb)
{
  if (!message.IsInitialized())
  {
    gzthrow("Simulator::SendMessage Message is not initialized[" + message.InitializationErrorString() + "]");
  }

  PublicationPtr pub = this->FindPublication(topic);

  pub->Publish( message, cb ); 
}

////////////////////////////////////////////////////////////////////////////////
PublicationPtr TopicManager::FindPublication(const std::string &topic)
{
  std::vector<PublicationPtr>::iterator iter;
  PublicationPtr pub;

  // Find the publication
  for (iter = this->advertisedTopics.begin(); 
      iter != this->advertisedTopics.end(); iter++)
  {
    if ((*iter)->GetTopic() == topic)
    {
      pub = *iter;
      break;
    }
  }

  return pub;
}

////////////////////////////////////////////////////////////////////////////////
// Subscribe to a topic give some options
SubscriberPtr TopicManager::Subscribe(const SubscribeOptions &ops)
{
  CallbackHelperPtr subscription = ops.GetSubscription();

  // Create a subscription (essentially a callback that gets 
  // fired every time a Publish occurs on the corresponding
  // topic
  //CallbackHelperPtr subscription( new CallbackHelperT<M>( callback ) );
  //this->subscribed_topics[topic].push_back(subscription);
  
  this->subscribed_topics[ops.GetTopic()].push_back(subscription);

  // The object that gets returned to the caller of this
  // function
  SubscriberPtr sub( new Subscriber(ops.GetTopic(), subscription) );

  // Find a current publication
  PublicationPtr pub = this->FindPublication(ops.GetTopic());

  // If the publication exits, just add the subscription to it 
  if (pub)
  {
    pub->AddSubscription( subscription );
  }
  else
  {
    // Otherwise subscribe to the remote topic
    ConnectionManager::Instance()->Subscribe(ops.GetTopic(), ops.GetMsgType());
  }

  return sub;
}


////////////////////////////////////////////////////////////////////////////////
// Handle an incoming message
void TopicManager::HandleIncoming()
{
  //implement this
  // Read a header in the message the indicates the topic
}

////////////////////////////////////////////////////////////////////////////////
// Unsubscribe from a topic
void TopicManager::Unsubscribe( const std::string &topic, CallbackHelperPtr sub)
{
  PublicationPtr publication = this->FindPublication(topic);
  if (publication)
  {
    publication->RemoveSubscription(sub);
  }

  this->subscribed_topics[topic].remove( sub );
}

////////////////////////////////////////////////////////////////////////////////
// Connect a local Publisher to a remote Subscriber
void TopicManager::ConnectPubToSub( const std::string &topic,
                                    const SubscriptionTransportPtr &sublink )
{
  PublicationPtr publication = this->FindPublication( topic );
  publication->AddSubscription( sublink );
}

////////////////////////////////////////////////////////////////////////////////
// Disconnect a local publisher from a remote subscriber
void TopicManager::DisconnectPubFromSub( const std::string &topic, const std::string &host, unsigned int port)
{
  PublicationPtr publication = this->FindPublication(topic);
  publication->RemoveSubscription(host, port);
}

////////////////////////////////////////////////////////////////////////////////
// Disconnection all local subscribers from a remote publisher
void TopicManager::DisconnectSubFromPub( const std::string &topic, const std::string &host, unsigned int port)
{
  PublicationPtr publication = this->FindPublication(topic);
  publication->RemoveTransport(host, port);
}

////////////////////////////////////////////////////////////////////////////////
// Connect all subscribers on a topic to known publishers
void TopicManager::ConnectSubscibers(const std::string &topic)
{
  SubMap::iterator iter = this->subscribed_topics.find(topic);

  if (iter != this->subscribed_topics.end())
  {
    PublicationPtr publication = this->FindPublication(topic);

    // Add all of our subscriptions to the publication
    std::list<CallbackHelperPtr>::iterator cbIter;
    for (cbIter = iter->second.begin(); cbIter != iter->second.end(); cbIter++)
    {
      publication->AddSubscription( *cbIter );
    }
  }
  else
    gzerr << "Shouldn't get here\n";//TODO: Properly handle this error
}

////////////////////////////////////////////////////////////////////////////////
/// Connect a local subscriber to a remote publisher
void TopicManager::ConnectSubToPub( const std::string &topic,
                                    const PublicationTransportPtr &publink )
{
  // Add the publication transport mechanism to the publication.
  if (publink)
  {
    PublicationPtr publication = this->FindPublication(topic);
    if (publication)
      publication->AddTransport( publink );
    else
      gzerr << "Attempting to connect a remote publisher...but we don't have a publication. This shouldn't happen\n";
  }

  this->ConnectSubscibers(topic);
}


////////////////////////////////////////////////////////////////////////////////
// Add a new publication to the list of advertised publication
bool TopicManager::UpdatePublications( const std::string &topic, 
                                       const std::string &msgType )
{
  bool inserted = false;

  // Find a current publication on this topic
  PublicationPtr pub = this->FindPublication(topic);

  if (pub)
  {
    // TODO: Handle this error properly
    if (msgType != pub->GetMsgType())
      gzerr << "Attempting to advertise on an existing topic with a conflicting message type\n";
  }
  else
  {
    inserted = true;
    pub = PublicationPtr( new Publication(topic, msgType) );
    this->advertisedTopics.push_back( pub );
  }

  return inserted;
}

////////////////////////////////////////////////////////////////////////////////
/// Stop advertising on a topic
void TopicManager::Unadvertise(const std::string &topic)
{
  int count = 0;
  std::vector<PublicationPtr>::iterator iter;

  for (iter = this->advertisedTopics.begin(); 
       iter != this->advertisedTopics.end(); iter++)
  {
    if ((*iter)->GetTopic() == topic)
      count++;
  }

  // Tell the master we are 
  if (count <= 1)
  {
    ConnectionManager::Instance()->Unadvertise(topic);
  }
}
