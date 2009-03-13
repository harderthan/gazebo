#include <gazebo/gazebo.h>
#include <gazebo/GazeboError.hh>

gazebo::Client *client = NULL;
gazebo::SimulationIface *simIface = NULL;
gazebo::AudioIface *audioIface = NULL;

int main()
{
  client = new gazebo::Client();
  simIface = new gazebo::SimulationIface();
  audioIface = new gazebo::AudioIface();

  int serverId = 0;

  /// Connect to the libgazebo server
  try
  {
    client->ConnectWait(serverId, GZ_CLIENT_ID_USER_FIRST);
  }
  catch (gazebo::GazeboError e)
  {
    std::cout << "Gazebo error: Unable to connect\n" << e << "\n";
    return -1;
  }

  /// Open the Simulation Interface
  try
  {
    simIface->Open(client, "default");
  }
  catch (gazebo::GazeboError e)
  {
    std::cout << "Gazebo error: Unable to connect to the sim interface\n" << e << "\n";
    return -1;
  }

  /// Open the OpenAL interface
  try
  {
    audioIface->Open(client, "audio_iface_1");
  }
  catch (std::string e)
  {
    std::cout << "Gazebo error: Unable to connect to the audio interface\n"
    << e << "\n";
    return -1;
  }

  printf("Play\n");
  /// Play the sound in the buffer
  audioIface->Lock(1);
  audioIface->data->cmd_play = 1;
  audioIface->Unlock();

  usleep(1000000);

  printf("Pause\n");
  /// Pause the sound
  audioIface->Lock(1);
  audioIface->data->cmd_stop = 1;
  audioIface->Unlock();

  usleep(1000000);

  printf("Continue\n");

  /// Play the sound in the buffer
  /*audioIface->Lock(1);
  audioIface->data->cmd_play = 1;
  audioIface->Unlock();
  */

  return 0;
}


