simple Generator
{
    parameters:
        volatile int packetByteSize;
        volatile double generationInterval; // sec
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile int bufferSize;
        volatile double serviceTime; // sec
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

simple TransportTx
{
    parameters:
        volatile int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}

module NodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        output out;
        input in;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        traTx: TransportTx {
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> traTx.toApp;
        in --> traTx.toOut$i;
        traTx.toOut$o --> out;
}
simple TransportRx
{
    parameters:
        volatile int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}
module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        traRx: TransportRx {
            parameters:
                
                @display("p=75,190");
        }
    connections:
        in --> traRx.toOut$i;
        //case 1:
        traRx.toApp --> {  datarate = 0.5Mbps; } --> sink.in;
        //case study 2:
        //traRx.toApp --> { datarate = 1Mbps; } --> sink.in;
        traRx.toOut$o --> out;
}

network Network
{
    @display("bgl=6");
    submodules:
        nodeTx: NodeTx {
            @display("p=23,30");
        }
        queue: Queue {
            @display("p=138,30");
        }
        queue1: Queue {
            @display("p=138,131");
        }
        nodeRx: NodeRx {
            @display("p=230,30");
        }
    connections:
        
        nodeTx.out --> {  datarate = 1Mbps; delay = 100us; } --> queue.in;
        //case study 1:
        queue.out --> {  datarate = 1Mbps; delay = 100us; } --> nodeRx.in;
        //case study 2:
        //queue.out --> { datarate = 0.5Mbps; delay = 100us; } --> nodeRx.in;
        nodeRx.out --> {  datarate = 1Mbps; delay = 100us; } --> queue1.in;
        queue1.out --> nodeTx.in;
 
        
}

