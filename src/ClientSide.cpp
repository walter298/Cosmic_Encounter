#include "ClientSide.h"

void load() {

}

void play(const tcp::endpoint& serverEndpoint, const std::string& name)
{
	nv::NovalisInstance instance{ "Cosmic Encounter" };

	instance.loadObjsFromDir(nv::workingDirectory() + "/objects");
	associateCardRenders(instance);

	//associateCardRenders(instance)
	asio::io_context context;
	
	//prevent context.run() from immediately returning
	asio::io_context::work work{ context };
	
	//joins at end of function
	asio::thread_pool threadPool;
	
	asio::post(threadPool, 
		[&context] { context.run(); }
	);

	Player p{ context, name };

	WaitingRoom waitingRoom{ instance, context, p, serverEndpoint };
	waitingRoom.execute();

	if (waitingRoom.endReason() == nv::Scene::EndReason::Quit) {
		context.stop();
		threadPool.join();
		return;
	}

	CardSelection selection{ instance, p.hand };
	selection.execute();

	context.stop();
	threadPool.join();
}