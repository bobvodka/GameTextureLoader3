// Handles allocating decoder objects to decode the stream of data for an image file

#include "decoderRegistry.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../decoders/DefaultGTLDecoders.hpp"

namespace GTLCore
{
	namespace mi = boost::multi_index;

	struct DecoderCreatorDetails
	{
		GameTextureLoader3::FileTypes id_;
		std::string ext_;
		GameTextureLoader3::DecoderCreator_t creator_;

		DecoderCreatorDetails(GameTextureLoader3::FileTypes id, std::string const &ext, GameTextureLoader3::DecoderCreator_t creator) : id_(id), ext_(ext), creator_(creator)
		{
		}
		DecoderCreatorDetails(DecoderCreatorDetails const &lhs) : id_(lhs.id_), ext_(lhs.ext_), creator_(lhs.creator_)
		{
		}
		bool operator<(DecoderCreatorDetails const &lhs) const 
		{ 
			return id_ < lhs.id_;
		};
	};

	struct extension{};
	struct id{};

	typedef mi::multi_index_container<
		DecoderCreatorDetails,
		mi::indexed_by<
			mi::ordered_unique< mi::tag<id>, mi::member<DecoderCreatorDetails,GameTextureLoader3::FileTypes, &DecoderCreatorDetails::id_> >,
			mi::ordered_non_unique< mi::tag<extension>, mi::member<DecoderCreatorDetails,std::string, &DecoderCreatorDetails::ext_> > 
		> 
	> decodermap_t;

	class DecoderRegistry
	{
	public:
		DecoderRegistry()
		{
			nextDecoderID = 1;
		}
		~DecoderRegistry()
		{

		}
		
		void RegisterDecoder(GameTextureLoader3::FileTypes id, std::string const &key, GameTextureLoader3::DecoderCreator_t creator)
		{
			RegisterDecoderImpl(id,key,creator);
		}
		GameTextureLoader3::FileTypes RegisterDecoder(std::string const &key, GameTextureLoader3::DecoderCreator_t creator)
		{
			RegisterDecoderImpl(nextDecoderID,key,creator);
			return nextDecoderID;
		}

		template<typename key, typename T>
		GameTextureLoader3::DecoderCreator_t getCreator(T const &val)
		{
			typedef typename decodermap_t::index<key>::type tex_by_key;
			tex_by_key &name_idx = imageDataDecoders.get<key>();

			tex_by_key::iterator it = name_idx.find(val);
			if(it == name_idx.end())
				throw GameTextureLoader3::DecoderNotFoundException("Texture Loader not found for texture of type : " + boost::lexical_cast<std::string>(val));
			
			return (*it).creator_;
		}
	protected:
	private:
		void RegisterDecoderImpl(GameTextureLoader3::FileTypes id, std::string const &key, GameTextureLoader3::DecoderCreator_t creator)
		{
			std::string capskey;
			std::transform(key.begin(), key.end(),std::back_inserter(capskey),toupper);
			imageDataDecoders.insert(DecoderCreatorDetails(id,capskey,creator));
			++nextDecoderID;
		}
		decodermap_t imageDataDecoders;
		int nextDecoderID;
	};

	DecoderRegistry registry;

	// Load up the Decoder Registry with the decoders we know about and any user defined ones
	GameTextureLoader3::extensionToFileTypesMap_t RegisterDecoders(GameTextureLoader3::userDecoderDetailsVec_t const &userdecoders)
	{
		registry.RegisterDecoder(GameTextureLoader3::TYPE_BMP,"bmp",Decoders::CreateBitmapDecoder);
		registry.RegisterDecoder(GameTextureLoader3::TYPE_JPG,"jpg",Decoders::CreateJPGDecoder);
		registry.RegisterDecoder(GameTextureLoader3::TYPE_TGA,"tga",Decoders::CreateTGADecoder);
		registry.RegisterDecoder(GameTextureLoader3::TYPE_PNG,"png",Decoders::CreatePNGDecoder);
		registry.RegisterDecoder(GameTextureLoader3::TYPE_DDS,"dds",Decoders::CreateDDSDecoder);

		GameTextureLoader3::extensionToFileTypesMap_t userCreators;

		if(!userdecoders.empty())
		{
			typedef GameTextureLoader3::userDecoderDetailsVec_t::const_iterator itr;
			for(itr it = userdecoders.begin(); it != userdecoders.end(); ++it)
			{
				GameTextureLoader3::FileTypes id = registry.RegisterDecoder(it->extension,it->decoderCreator);
				userCreators.insert(std::make_pair(it->extension, id));
			}
		}
		return userCreators;
	}

	GameTextureLoader3::DecoderCreator_t getCreator(std::string const &ext)
	{
		return registry.getCreator<extension>(ext);
	}
	
	GameTextureLoader3::DecoderCreator_t getCreator(GameTextureLoader3::FileTypes ext)
	{
		return registry.getCreator<id>(ext);
	}

}