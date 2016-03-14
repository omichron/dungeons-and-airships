#include "TMXParser.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
//#include "base64.h"

namespace TMX 
{
	Parser::Parser( const char* filename )
	{
	load( filename );
	}

	Parser::Parser()
	{
	}

	Parser::~Parser()
	{
	}

	const char* Parser::LoadElement(const char* elementName, rapidxml::xml_node<>* root_node) const
	{
		if (!root_node) return nullptr;
		auto first_attribute = root_node->first_attribute(elementName);
		if (!first_attribute) return nullptr;
		auto value = first_attribute->value();
		if (!value) return nullptr;
		return value;
	}

	template<> 
	std::string Parser::LoadElement<std::string>(const char* elementName, rapidxml::xml_node<>* root_node) const
	{
		auto value = LoadElement(elementName, root_node);
		return value ? value : "";
	}

	template<>
	int Parser::LoadElement<int>(const char* elementName, rapidxml::xml_node<>* root_node) const
	{
		auto value = LoadElement(elementName, root_node);
		return value ? atoi(value) : 0;
	}

	template<>
	bool Parser::LoadElement<bool>(const char* elementName, rapidxml::xml_node<>* root_node) const
	{
		auto value = LoadElement(elementName, root_node);
		return LoadElement<int>(elementName, root_node) != 0;
	}

	template<>
	float Parser::LoadElement<float>(const char* elementName, rapidxml::xml_node<>* root_node) const
	{
		auto value = LoadElement(elementName, root_node);
		return value ? atof(value) : 0;
	}

	//TODO: Sanity
	bool Parser::load( const char* filename )
	{
		std::string version = VERSION;
	
		rapidxml::xml_document<> doc;
		rapidxml::file<> file( filename );
		doc.parse<0>( file.data() );
		//get root nodes
		rapidxml::xml_node<>* root_node = doc.first_node( "map" );

		//load map element
		if( root_node->first_attribute( "version" )->value() != version ) 
		{
			std::cout << "ERROR: Map version mismatch. Required version: " << VERSION << "." << std::endl;
			return false;
		}

		mapInfo.version = LoadElement<std::string>("version", root_node);
		mapInfo.orientation = LoadElement<std::string>("orientation", root_node);
		mapInfo.width = LoadElement<int>("width", root_node);
		mapInfo.height = LoadElement<int>("height", root_node);
		mapInfo.tileWidth = LoadElement<int>("tilewidth", root_node);
		mapInfo.tileHeight = LoadElement<int>("tileheight", root_node);
		mapInfo.backgroundColor = LoadElement<std::string>("backgroundcolor", root_node);


		if( root_node->first_node( "properties" ) != 0 ) 
		{
			for( rapidxml::xml_node<>* properties_node = root_node->first_node( "properties" )->first_node( "property" ); properties_node; properties_node = properties_node->next_sibling() ) 
			{
				mapInfo.property[properties_node->first_attribute( "name" )->value()] = properties_node->first_attribute( "value" )->value();
			}
		}

		for( rapidxml::xml_node<>* tileset_node = root_node->first_node( "tileset" ); tileset_node; tileset_node = tileset_node->next_sibling( "tileset" ) ) 
		{
			Tileset tmpTileset;
			tmpTileset.firstGID = LoadElement<int>("firstgid", tileset_node);
			tmpTileset.source = LoadElement<std::string>("source", tileset_node);
			tmpTileset.name = LoadElement<std::string>("name", tileset_node);
			tmpTileset.tilewidth = LoadElement<int>("tilewidth", tileset_node);
			tmpTileset.tileheight = LoadElement<int>("tileheight", tileset_node);
			tmpTileset.tilecount = LoadElement<int>("tilecount", tileset_node);
			tmpTileset.columns = LoadElement<int>("columns", tileset_node);
			tilesetList.push_back(tmpTileset);
		}

		for( rapidxml::xml_node<>* layer_node = root_node->first_node( "layer" ); layer_node; layer_node = layer_node->next_sibling( "layer" ) ) 
		{
			TileLayer layer;
			layer.name = LoadElement<std::string>("name", layer_node);
			layer.width = LoadElement<int>("width", layer_node);
			layer.height = LoadElement<int>("height", layer_node);

			if( layer_node->first_node( "properties" ) != 0 ) 
			{
				for( rapidxml::xml_node<>* properties_node = layer_node->first_node( "properties" )->first_node( "property" ); properties_node; properties_node = properties_node->next_sibling() ) 
				{
					layer.property[LoadElement<std::string>("name", properties_node)] = LoadElement<std::string>("value", properties_node);
				}
			}

			rapidxml::xml_node<>* data_node = layer_node->first_node( "data" );
			layer.data.encoding = LoadElement<std::string>("encoding", data_node);
			layer.data.compression = LoadElement<std::string>("compression", data_node);
			layer.data.contents = data_node->value();
			tileLayer[layer.name] = layer;
		}

		for( rapidxml::xml_node<>* oGroup_node = root_node->first_node( "objectgroup" ); oGroup_node; oGroup_node = oGroup_node->next_sibling( "objectgroup" ) ) 
		{
			ObjectGroup oGroup;
			oGroup.color = LoadElement<std::string>("color", oGroup_node);
			oGroup.name = LoadElement<std::string>("name", oGroup_node);
			oGroup.opacity = LoadElement<float>("opacity", oGroup_node);
			oGroup.visible = LoadElement<bool>("visible", oGroup_node);

			if( oGroup_node->first_node( "properties" ) != 0 ) 
			{
				for( rapidxml::xml_node<>* properties_node = oGroup_node->first_node( "properties" )->first_node( "property" ); properties_node; properties_node = properties_node->next_sibling() ) 
				{
					oGroup.property[LoadElement<std::string>("name", properties_node)] = LoadElement<std::string>("value", properties_node);
				}
			}
			objectGroup[oGroup.name] = oGroup;
		}

		for( rapidxml::xml_node<>* image_node = root_node->first_node( "imagelayer" ); image_node; image_node = image_node->next_sibling( "imagelayer" ) ) 
		{
			ImageLayer imgLayer;
			imgLayer.name = LoadElement<std::string>("name", image_node);
			imgLayer.opacity = LoadElement<float>("opacity", image_node);
			imgLayer.visible = LoadElement<bool>("visible", image_node);
			imgLayer.image.source = LoadElement<std::string>("source", image_node->first_node("image"));
			imgLayer.image.transparencyColor = LoadElement<std::string>("trans", image_node->first_node("image"));

			if( image_node->first_node( "properties" ) != 0 ) 
			{
				for( rapidxml::xml_node<>* properties_node = image_node->first_node( "properties" )->first_node( "property" ); properties_node; properties_node = properties_node->next_sibling( "property" ) ) 
				{
					imgLayer.property[LoadElement<std::string>("name", properties_node)] = LoadElement<std::string>("value", properties_node);
				}
			}
			imageLayer[imgLayer.name] = imgLayer;
		}
		return true;
	}
}
