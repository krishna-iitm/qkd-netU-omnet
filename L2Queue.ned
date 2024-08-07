//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Represents the network interface for a point-to-point connection.
//
// Packets arriving from the upper layer ("in" gate) are sent out on
// the "line" gate, which is expected to be connected to a link with
// nonzero data rate. Packets that arrive while a previous packet is
// being transmitted are queued up. The maximum queue size in packets
// can be specified in a parameter. Excess frames are simply discarded
// and recorded as statistics.
//
// The model can be easily extended in several ways: to make it possible to
// query the queue length from another module via a direct method call
// interface, to implement priority queueing, or to collect link statistics
// (utilization, etc.)
//
simple L2Queue 
{
    parameters:
        int QueueCapacity = 100000;//default(0); // max number of packets; 0 means no limit
        int linkCapacity;
        double timeSlot;
        bool useCutThroughSwitching = default(false);  // use cut-through switching instead of store-and-forward
        @display("i=block/queue;q=queue");
        int TotalnumberOfSlot;
        @signal[qlen](type="long");
         @signal[xQlenSignal](type="long");
         @signal[yQlenSignal](type="long");
//         @signal[sumQlen](type="double");
//        @signal[busy](type="bool");
     //     @signal[queueingTime](type="simtime_t");
          @signal[drop](type="long");
//        @signal[txBytes](type="long");
//        @signal[rxBytes](type="long");
//         @statistic[qlen](title="queue length";timeavg,max,vector;interpolationmode=sample-hold);
           @statistic[physicalQlenX](source=xQlenSignal;record=timeavg,max,mean, vector,last);
           @statistic[physicalQlenY](source=yQlenSignal;record=timeavg,max,mean, vector,last);
           @statistic[sumQlen](source=(xQlenSignal+yQlenSignal);title="X+Y";record= last);
//        @statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);
   //       @statistic[queueingTime](title="queueing time at dequeue";unit=s; record=timeavg,max,mean;interpolationmode=none);
//        @statistic[drop](title="dropped packet byte length";unit=bytes;record=count,sum;interpolationmode=none);
    //      @statistic[drop](title="dropped packet byte length";unit=bytes;record=vector?,count,sum;interpolationmode=none);
//        @statistic[txBytes](title="transmitting packet byte length";unit=bytes;record=vector?,count,sum,histogram;interpolationmode=none);
//        @statistic[rxBytes](title="received packet byte length";unit=bytes;record=vector?,count,sum,histogram;interpolationmode=none);
    gates:
        input in;
        output out;
        inout line;
}


