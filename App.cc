
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include <map>
#include <omnetpp.h>
#include "Packet_m.h"
#include <string.h>
using namespace std;
using namespace omnetpp;


class App : public cSimpleModule
{
  private:

    int myAddress;  //Address of the parent node i.e., index of the node
    int numOfNodes; // Number of nodes in the network (user defined value)
    double packeSizeBytes; // Size of each packet
    int sourceNode;
    int destinationNode;

    double timeSlotDuration; // duration of each time slot in seconds (user defined value)
    double lambda; // lamda for poisson distribution (user defined value)
    int PacketArrivalPerSlot; // number of packets generated per time slot depending on the value of lambda
    int TotaltimeSlot; // Total simulation time in terms of time slot (user defined value)
    int timeSlotCounter; // Counter to count the number of slots completed in order to reach the total time slot.
    int traffic_class_index; // used to select the traffic class (user defined value)
    long totalNumOfUnicastPacketsGenerated;
    typedef std::map<int, std::string> trafficType;  //mapping: key is traffic_class_index and value is the name of traffic class (String)
    trafficType setTrafficType;

    typedef std::map<int, int> SourceDestinationPair;
    SourceDestinationPair S_and_D_Pair;

    cMessage *generatePacket; // message object to generate the packets
    cTopology *getTopoInfo;

    // signals to record the statistic endToEndDelayForUnEncryptedPckts
    simsignal_t endToEndDelayForUnEncryptedPcktsSignal; // used to record the end to end delay of packet received at the destination node
    simsignal_t endToEndDelayForEncryptedPcktsSignal; // used to record the end to end delay of packet received at the destination node
    simsignal_t totalDelayByQueue_X_Signal; //
    simsignal_t totalDelayByQueue_Y_Signal; //
    simsignal_t totalPropagationDelaySignal; //
    simsignal_t hopCountSignal; // used to record the hop count of the packet received at the destination node


  public:
    App(); // Class Constructor
    virtual ~App(); // Class Destructor

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void generateUnicastPackt(string trafficClass, int totalPacketsArrivedInCurrentSlot);
    void generateBroadcastPacket(string trafficClass, int totalPacketsArrivedInCurrentSlot);
    virtual void finish();
};

Define_Module(App);

App::App()
{
    generatePacket = nullptr;
}

App::~App()
{
    cancelAndDelete(generatePacket);
    delete getTopoInfo;
}

void App::initialize()
{
    numOfNodes = par("n");
    timeSlotDuration = par("timeSlot");
    TotaltimeSlot = par("TotalnumberOfSlot");
    traffic_class_index = par("Traffic_Class");
    myAddress = par("address");
    lambda = par("lambda");
 //   sourceNode = par("source_node");
 //   destinationNode = par("destination_node");
    timeSlotCounter=0;
    packeSizeBytes = par("packetSize");
    totalNumOfUnicastPacketsGenerated =0;
    setTrafficType.insert(std::pair<int, string>(1, "Unicast"));
    setTrafficType.insert(std::pair<int, std::string>(2, "Broadcast"));
    setTrafficType.insert(std::pair<int, std::string>(3, "Mixed"));

//    S_and_D_Pair.insert(pair<int, int>(1, 9));
//   S_and_D_Pair.insert(pair<int, int>(5, 14));
//    S_and_D_Pair.insert(pair<int, int>(11, 6));
//   S_and_D_Pair.insert(pair<int, int>(3, 10));
//   S_and_D_Pair.insert(pair<int, int>(8, 13));
//   S_and_D_Pair.insert(pair<int, int>(16, 8));
//   S_and_D_Pair.insert(pair<int, int>(6, 28));
//   S_and_D_Pair.insert(pair<int, int>(32, 2));
//   S_and_D_Pair.insert(pair<int, int>(7, 33));
//   S_and_D_Pair.insert(pair<int, int>(13, 30));

    S_and_D_Pair.insert(pair<int, int>(1, 4));
    //S_and_D_Pair.insert(pair<int, int>(8, 5));
//    S_and_D_Pair.insert(pair<int, int>(3, 2));
    //S_and_D_Pair.insert(pair<int, int>(9, 6));
    //S_and_D_Pair.insert(pair<int, int>(7, 0));

    map<int, int>::iterator it ;
    it = S_and_D_Pair.find(myAddress);
    if(it == S_and_D_Pair.end()){
        EV << "Key-value pair not present in map\n" ;
        sourceNode = -1;
        destinationNode = -1;
    }
    else
    {
        EV << "Key-value pair present\n ";

        sourceNode = it->first;
        destinationNode = it->second;
        EV<<" Source address is "<<sourceNode<<endl;
        EV<<" Destination address is "<<destinationNode<<endl;
    }


//
//    S_and_D_Pair.insert(pair<int, int>(14, 24));
//    S_and_D_Pair.insert(pair<int, int>(8, 22));




    getTopoInfo = new cTopology("Topology finding at APP module");
    std::vector<std::string> nedTypes;
    nedTypes.push_back(getParentModule()->getNedTypeName());
    getTopoInfo->extractByNedTypeName(nedTypes);
    cTopology::Node *node = getTopoInfo->getNodeFor(getParentModule());

    if(node->getNumOutLinks()>=1)
    {

            if (myAddress == sourceNode)
            {
                generatePacket = new cMessage("nextPacket");
                scheduleAt(timeSlotDuration, generatePacket);
            }
    }


    endToEndDelayForUnEncryptedPcktsSignal = registerSignal("endToEndDelayForUnEncryptedPckts");
    endToEndDelayForEncryptedPcktsSignal = registerSignal("endToEndDelayForEncryptedPckts");
    totalDelayByQueue_X_Signal = registerSignal("totalDelayByQueue_X");
    totalDelayByQueue_Y_Signal = registerSignal("totalDelayByQueue_Y");
    totalPropagationDelaySignal = registerSignal("totalPropagationDelay");
    hopCountSignal = registerSignal("hopCount");


}

void App::handleMessage(cMessage *msg)
{
    if (msg == generatePacket)
    {
        if (timeSlotCounter<TotaltimeSlot)
           {
            trafficType::iterator it = setTrafficType.find(traffic_class_index);
            string traffic_Class = (*it).second;
            EV<< "Selected Traffic class is: "<<traffic_Class<<endl;
            PacketArrivalPerSlot = poisson(lambda);//poisson(lambda);2;

            if(strcmp(traffic_Class.c_str(), "Unicast")==0)
            {
             EV<<"Number of unicast packets generated in the time slot: "<<timeSlotCounter<< " are "<<PacketArrivalPerSlot<<endl;
             generateUnicastPackt(traffic_Class,PacketArrivalPerSlot);
             totalNumOfUnicastPacketsGenerated +=PacketArrivalPerSlot;
            }

            if(strcmp(traffic_Class.c_str(), "Broadcast")==0)
            {
             EV<<"Number of broadcast packets generated in the time slot: "<<timeSlotCounter<< " are "<<PacketArrivalPerSlot<<endl;
             generateBroadcastPacket(traffic_Class,PacketArrivalPerSlot);
            }

            if(strcmp(traffic_Class.c_str(), "Mixed")==0) // broadCast traffic generation
            {
             EV<<"Number of mixed (unicast plus broadcast) packets generated in the time slot: "<<timeSlotCounter<< " are "<<PacketArrivalPerSlot<<endl;
             int num_of_unicast_pckt = intuniform(0,PacketArrivalPerSlot);
             EV<<"Among "<<PacketArrivalPerSlot<<" packets, number of unicast packets are: "<<num_of_unicast_pckt<<endl;
             int num_of_broadcast_pckt = PacketArrivalPerSlot - num_of_unicast_pckt;
             EV<<"and number of broadcast packets are: "<<num_of_broadcast_pckt<<endl;
             generateUnicastPackt(traffic_Class,num_of_unicast_pckt);
             generateBroadcastPacket(traffic_Class,num_of_broadcast_pckt);
            }

        timeSlotCounter++;
        scheduleAt(simTime() + timeSlotDuration, generatePacket);
           }
   }
    // Handle incoming packet
    else
    {
        Packet *pk = check_and_cast<Packet *>(msg);
        EV << "received packet " << pk->getName() << " after " << pk->getHopCount() << " hops" << " from "<<pk->getSrcAddr()<< " to "<< myAddress<<endl;
        if (pk->getEnableEncryption())
        {
            emit(endToEndDelayForEncryptedPcktsSignal, simTime() - pk->getCreationTime());
            emit(totalDelayByQueue_X_Signal, pk->getTotalQueueDelay_X());
            emit(totalDelayByQueue_Y_Signal, pk->getTotalQueueDelay_Y());
            emit(totalPropagationDelaySignal, (pk->getHopCount()*0.025));
            emit(hopCountSignal,(pk->getHopCount()));

            EV<<"Total end to end delay for the packet that need quantum encryption is "<<simTime() - pk->getCreationTime()<<endl;
            EV<<"Total delay due to queue X is "<<pk->getTotalQueueDelay_X()<<endl;
            EV<<"Total delay due to queue Y is "<<pk->getTotalQueueDelay_Y()<<endl;
            EV<<"Total propagation delay is "<<(pk->getHopCount()*0.025)<<endl;

        }

        if (pk->getEnableEncryption()==0)
        {
            emit(endToEndDelayForUnEncryptedPcktsSignal, simTime() - pk->getCreationTime());
            EV<<"Total end to end delay for the packet don't that need quantum encryption is "<<simTime() - pk->getCreationTime()<<endl;
        }


        EV<<"The received packet was generated in the time slot "<<pk->getTimeSlotCounter()<<endl;
        delete pk;
        if (hasGUI())
        getParentModule()->bubble("Arrived!");
    }
}

void App :: generateBroadcastPacket(string traffic_Class, int Num_of_packet)
{
    if(Num_of_packet>=1)
    {
        if (hasGUI())
            getParentModule()->bubble("Generating Broadcast packet...");
    }

    for(int i=0; i<Num_of_packet;i++)
    {
        char pkname[40];
        sprintf(pkname, "broadCast-pk-%d-to-all", myAddress);
        EV << "Generated packet name is " << pkname << endl;
        Packet *pk = new Packet("BroadCast Packet");
        pk->setByteLength(packeSizeBytes);
        pk->setKind(intuniform(0, 7));
        pk->setSrcAddr(sourceNode);
        pk->setTotalQueueDelay_X(0);
        pk->setTotalQueueDelay_Y(0);
        pk->setEnableEncryption(true);
        pk->setPacketName(pkname);
        pk->setTimeSlotCounter(timeSlotCounter);
        send(pk, "out");
    }

}

void App :: generateUnicastPackt(string traffic_Class, int Num_of_packet)
{
    if(Num_of_packet>=1)
    {
        if (hasGUI())
            getParentModule()->bubble("Generating Unicast Packet...");
    }

    EV<<"total packet generated is "<<Num_of_packet<<endl;

    int tempNumberOfEncryptedPackets = Num_of_packet;// intuniform(0,Num_of_packet);
   // int tempNumberOfUnEncrytedPackets = Num_of_packet - tempNumberOfEncryptedPackets;

   // EV<<"total packets that need quantum encryption "<<tempNumberOfEncryptedPackets<<endl;
   // EV<<"total packets that don't need quantum encryption "<<tempNumberOfUnEncrytedPackets<<endl;

    for(int i=0; i<tempNumberOfEncryptedPackets;i++)
    {
//        int destAddress;
//        do
//        {
//            destAddress = 5;//intuniform(0, numOfNodes-1);//destinationNode;//intuniform(0, numOfNodes-1);
//        }
//        while (destAddress==myAddress);

        char pkname[40];
        sprintf(pkname, "uniCast-pk-%d-to-%d", sourceNode, destinationNode); // pkname is just for printing(not a packet data)
        EV << "Generated packet name is: " << pkname << endl;
        Packet *pk = new Packet("Unicast Packet");
        pk->setByteLength(packeSizeBytes);
        pk->setKind(intuniform(0, 7));
        pk->setSrcAddr(sourceNode);
        pk->setDestAddr(destinationNode);
        pk->setTotalQueueDelay_X(0);
        pk->setTotalQueueDelay_Y(0);
        pk->setEnableEncryption(true);
        pk->setPacketName(pkname);
        pk->setTimeSlotCounter(timeSlotCounter);
        send(pk, "out");
    }

//    for(int i=0; i<tempNumberOfUnEncrytedPackets;i++)
//    {
////        int destAddress;
////        do
////        {
////            destAddress = 5;//intuniform(0, numOfNodes-1);//destinationNode;//intuniform(0, numOfNodes-1);
////        }
////        while (destAddress==myAddress);
//
//        char pkname[40];
//        sprintf(pkname, "uniCast-pk-%d-to-%d", sourceNode, destinationNode);
//        EV << "Generated packet name is: " << pkname << endl;
//        Packet *pk = new Packet("Unicast Packet");
//        pk->setByteLength(packeSizeBytes);
//        pk->setEnableEncryption(false);
//        pk->setKind(intuniform(0, 7));
//        pk->setSrcAddr(sourceNode);
//        pk->setDestAddr(destinationNode);
//        pk->setTotalQueueDelay_X(0);
//        pk->setTotalQueueDelay_Y(0);
//        pk->setPacketName(pkname);
//        pk->setTimeSlotCounter(timeSlotCounter);
//        send(pk, "out");
//    }

}
void App::finish(){
    EV<<"#unicast pkts generated..."<<totalNumOfUnicastPacketsGenerated<<endl;
}
