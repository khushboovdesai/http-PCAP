define ($IP 10.10.1.1)  
define ($MAC 00:00:00:00:01:00)  

source::FromDevice;  
sink::ToDevice;  

c::Classifier(  


//APPLICATION LAYER PROTOCOL

	36/0050, 			//HTTP request
	-);  

source->c;  
	
c[0]->Print('HTTP Packet')  -> TCounter -> sink;  
c[1]->Print('Others')->Discard;
