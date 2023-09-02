#include "ClientSide.h"

void play(const tcp::endpoint& serverEndpoint, const std::string& name)
{
	nv::Instance instance{ "Cosmic Encounter" };

	instance.loadObjsFromDir(nv::workingDirectory() + "/objects");
	associateCardRenders(instance);
	std::println("Associated Card Renders\n");
	//asio::io_context context;

	//Client cli{ name, tcp::socket{ context } };

	//WaitingRoom waitingRoom{ instance, context, cli, serverEndpoint };

	////prevent context.run() from immediately returning
	//asio::io_context::work work{ context };

	////joins at end of function
	//asio::thread_pool threadPool;

	//asio::post(threadPool,
	//	[&context] { context.run(); }
	//);

	//waitingRoom.execute();

	//if (waitingRoom.endReason() == nv::Scene::EndReason::Quit) {
	//	context.stop();
	//	threadPool.join();
	//	return;
	//}

	//CardSelection selection{ instance, cli.p.hand };
	//selection.execute();

	//context.stop();
	//threadPool.join();
}