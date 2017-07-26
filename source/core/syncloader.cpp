// Sync. data loader function
#include "gtlcore.hpp"
#include "../../includes/gtl/ImageDecoderBase.hpp"

namespace GTLCore
{
	void LoadImageSync(WorkPacket *work)
	{
		work->Process();
		delete work;
	}
}

