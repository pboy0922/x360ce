#ifndef _INI_H_
#define _INI_H_

#include <clocale>
#include <map>
#include <vector>
#include <fstream>

class InI
{
public:

	InI();
	virtual ~InI(void);

	void open(const std::string filename);

	const std::string& currentSection();
	void useSection(const std::string& section);
	void unuseSection();

	const std::vector<std::string> getSections();
	const std::vector<std::string> getKeys(const std::string& section);
	const std::string getValue(const std::string& section, const std::string& key);
	const std::string getValue(const std::string& sectionkey);

	const long getValueAsLong(const std::string& keyname);
	const bool getValueAsBoolean(const std::string& keyname);

	void save(const std::string& filename);
	void setValue(const std::string& section, const std::string& key, const std::string& value);

	void deleteSection(const std::string& section);
	void deleteKey(const std::string& section, const std::string& key);

	void clearMap();

private:
	typedef std::map<std::string,std::string> KeyMap;
	typedef std::map<std::string,KeyMap> Inimap;

	Inimap ini_map;
	std::string selected_section;
	size_t bom;

	void prepare(std::ifstream& stream);
	void trim(std::string& str);
	void parse(std::ifstream& stream);

};

/*=======================================================================*/



inline InI::InI()
{

};

inline InI::~InI(void)
{

};

inline void InI::open(const std::string filename)
{
	std::ifstream inifile;
	inifile.open(filename,std::ios::text);

	if(!inifile.is_open() && !inifile.good() )
	{
		std::string e = "file read error";
		throw e;
	}

	prepare(inifile);
	parse(inifile);
	inifile.close();
}

inline const std::string& InI::currentSection()
{
	return selected_section;
}

inline void InI::useSection(const std::string& section)
{
	selected_section = section;
}

inline void InI::unuseSection()
{
	selected_section.clear();
	selected_section.shrink_to_fit();
}

inline const std::vector<std::string> InI::getSections()
{
	std::map <std::string, std::map <std::string, std::string> >::iterator sectionit;
	sectionit = ini_map.begin();

	std::vector<std::string> sections;

	for(sectionit; sectionit != ini_map.end(); ++sectionit)
	{
		sections.push_back(sectionit->first);
	}
	return sections;
}


inline const std::vector<std::string> InI::getKeys(const std::string& section)
{
	std::map <std::string, std::map <std::string, std::string> >::iterator sectionit;
	std::map <std::string, std::string>::iterator keyit;

	std::vector<std::string> keys;

	sectionit = ini_map.find(section);
	if(sectionit != ini_map.end())
	{
		keyit = sectionit->second.begin();
		for(keyit; keyit != sectionit->second.end(); ++keyit)
		{
			keys.push_back(keyit->first);
		}
	}
	return keys;
}


inline const std::string InI::getValue(const std::string& section, const std::string& key)
{
	if(ini_map.empty())
		return std::string();

	std::map <std::string, std::map <std::string, std::string> >::iterator sectionit;
	std::map <std::string, std::string>::iterator keyit;

	sectionit = ini_map.find(section);
	if(sectionit != ini_map.end())
	{
		keyit = sectionit->second.find(key);
		if(keyit != sectionit->second.end())
			return keyit->second;;
	}
	return std::string();
}

inline const std::string InI::getValue(const std::string& sectionkey)
{
	if(ini_map.empty())
		return std::string();

	std::string section;
	std::string key;

	if(sectionkey.find(".") != std::string::npos)
	{
		section = sectionkey.substr(0,sectionkey.find("."));
		key = sectionkey.substr(sectionkey.find(".")+1,sectionkey.length());
	}
	else if(selected_section.size() > 0)
	{
		section = selected_section;
		key = sectionkey;
	}
	else
		return std::string();

	return getValue(section,key);
}

inline const long InI::getValueAsLong(const std::string& keyname)
{
	return std::strtol(this->getValue(keyname).c_str(),NULL,0);
}

inline const bool InI::getValueAsBoolean(const std::string& keyname)
{
	return this->getValueAsLong(keyname) > 0;
}

inline void InI::save(const std::string& filename)
{
	std::ofstream inifile;	
	inifile.open(filename,std::ios::binary);
	
	//bom
	inifile.put((unsigned char)0xFF);
	inifile.put((unsigned char)0xFE);

	inifile.imbue(std::locale(inifile.getloc(),new std::codecvt_utf16<char, 1114111UL, std::little_endian>));
	inifile.flush();

	if(!inifile.is_open() && !inifile.good() )
	{
		std::string e = "file read error";
		throw e;
	}

	std::string saved = currentSection();

	std::vector<std::string> sections = getSections();
	for(std::string section : sections)
	{
		inifile << '[' << section << ']' << '\r' << std::endl;
		std::vector<std::string> keys = getKeys(section);
		for(std::string key : keys)
		{
			useSection(section);
			std::string val = getValue(key);
			inifile << key << '=' << val << '\r' << std::endl;
			unuseSection();
		}
		inifile << '\r' << std::endl;
	}

	inifile.close();
	useSection(saved);
}

inline void InI::setValue(const std::string& section, const std::string& key, const std::string& value)
{
	ini_map[section][key] = value;
}

inline void InI::deleteSection(const std::string& section)
{
	ini_map.erase(section);
}

inline void InI::deleteKey(const std::string& section, const std::string& key)
{
	ini_map[section].erase(key);
}

inline void InI::clearMap()
{
	ini_map.clear();
}

/*=======================================================================*/

inline void InI::prepare(std::ifstream& inifile)
{
	inifile.read((char*)&bom,3);
	bom &= 0xFFFFFF;

	if(bom == 0xEFBBBF) //UTF8
		inifile.imbue(std::locale(inifile.getloc(),new std::codecvt_utf8<char, 1114111UL>));
	else
	{
		inifile.seekg(2,std::ios::beg);
		bom &= 0xFFFF;
		
		if(bom == 0xFEFF) //UTF16LE
			inifile.imbue(std::locale(inifile.getloc(),new std::codecvt_utf16<char, 1114111UL, std::little_endian>));
		else if(bom == 0xFFFE) //UTF16BE
			inifile.imbue(std::locale(inifile.getloc(),new std::codecvt_utf16<char, 1114111UL>));
		else //ANSI
		{
			bom = 0;
			inifile.seekg(std::ios::beg);
		}
	}
}

inline void InI::trim(std::string& str)
{
	size_t pos = std::string::npos;

	if(bom)
	{
		pos = str.size()-1;
		if(pos != std::string::npos && str[pos] == '\r')
			str.erase(pos,1);
	}

	if(str.empty())
		return;

	pos = str.find('#');
	if( pos != std::string::npos)
		str.resize(pos);

	pos = str.find(';');
	if( pos != std::string::npos)
		str.resize(pos);

	str.erase(0, str.find_first_not_of(L' '));
	str.erase(str.find_last_not_of(L' ')+1);

	str.shrink_to_fit();
}

inline void InI::parse(std::ifstream& inifile)
{
	std::string section;
	std::string key;
	std::string val;

	while(inifile.good())
	{
		std::string line;
		getline(inifile,line);

		//fast skip comment
		if(line[0] == '#' || line[0] == ';')
			continue;

		trim(line);

		size_t sectionBeg = std::string::npos;
		size_t sectionEnd = std::string::npos;

		if ((sectionBeg = line.find('[')) != std::string::npos && (sectionEnd = line.find(']')) != std::string::npos)
			section = line.substr(sectionBeg+1,sectionEnd-1);
		else
		{
			size_t keyBeg = 0;
			size_t keyEnd = line.find('=');

			if(keyEnd != std::string::npos)
			{
				key = line.substr(keyBeg,keyEnd);

				size_t valBeg = keyEnd+1;
				size_t valEnd = line.size();

				val = line.substr(valBeg,valEnd);
				if(!val.empty())
					ini_map[section][key] = val;
			}
		}
	}
}

#endif