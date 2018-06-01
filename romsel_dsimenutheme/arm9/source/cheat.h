
#ifndef CHEAT_H
#define CHEAT_H

#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <stdint.h>

#define CHEAT_CODE_END	0xCF000000
#define CHEAT_ENGINE_RELOCATE	0xCF000001
#define CHEAT_ENGINE_HOOK	0xCF000002


typedef unsigned int CheatWord;

class CheatFolder;

class CheatBase 
{
private:
	CheatFolder* parent;
	
public:
	std::string name;
	std::string note;
	
	CheatBase (CheatFolder* parent)
	{
		this->parent = parent;
	}

	CheatBase (const char* name, CheatFolder* parent)
	{
		this->name = name;
		this->parent = parent;
	}
	
	CheatBase (const std::string name, CheatFolder* parent)
	{
		this->name = name;
		this->parent = parent;
	}
	
	virtual ~CheatBase () 
	{
	}
	
	const char* getName (void)
	{
		return name.c_str();
	}
	
	const char* getNote (void)
	{
		return note.c_str();
	}
	
	CheatFolder* getParent (void)
	{
		return parent;
	}
	
	virtual std::list<CheatWord> getEnabledCodeData(void)
	{
		std::list<CheatWord> codeData;
		return codeData;
	}
} ;

class CheatCode : public CheatBase 
{
public:
	CheatCode (CheatFolder* parent) : CheatBase (parent)
	{
		enabled = false;
		always_on = false;
		master = false;
	}

	void setEnabled (bool enable) 
	{
		if (!always_on) {
			enabled = enable;
		}
	}
	
	void toggleEnabled (void);
	
	bool getEnabledStatus (void) 
	{
		return enabled;
	}
	
	bool isMaster (void)
	{
		return master;
	}
	
	std::list<CheatWord> getCodeData(void)
	{
		return cheatData;
	}
	
	void setCodeData (const std::string& data);
	
	std::list<CheatWord> getEnabledCodeData(void);
	
	static const int CODE_WORD_LEN = 8;
	
private:
	std::list<CheatWord> cheatData;
	bool enabled;
	bool always_on;
	bool master;
} ;

class CheatFolder : public CheatBase
{
public:
	CheatFolder (const char* name, CheatFolder* parent) : CheatBase (name, parent)
	{
		allowOneOnly = false;
	}
	
	CheatFolder (CheatFolder* parent) : CheatBase (parent)
	{
		allowOneOnly = false;
	}

	~CheatFolder();
	
	void addItem (CheatBase* item)
	{
		if (item) {
			contents.push_back(item);
		}
	}
	
	void enablingSubCode (void);

	void enableAll (bool enabled);
	
	void setAllowOneOnly (bool value) 
	{
		allowOneOnly = value;
	}
	
	std::vector<CheatBase*> getContents(void) 
	{
		return contents;
	}
	
	std::list<CheatWord> getEnabledCodeData(void);
	
protected:
	std::vector<CheatBase*> contents;

private:
	bool allowOneOnly;
	
} ;

class CheatGame : public CheatFolder
{
public:
	CheatGame (const char* name, CheatFolder* parent) : CheatFolder (name, parent)
	{
		memset(gameid, ' ', 4);
	}
	
	CheatGame (CheatFolder* parent) : CheatFolder (parent)
	{
		memset(gameid, ' ', 4);
	}
	
	bool checkGameid (const char gameid[4], uint32_t headerCRC)
	{
		return (memcmp (gameid, this->gameid, sizeof(this->gameid)) == 0) &&
			(headerCRC == this->headerCRC);
	}

	void setGameid (const std::string& id);

private:
	char gameid[4];
	uint32_t  headerCRC;
} ;

class CheatCodelist : public CheatFolder 
{
public:
	CheatCodelist (void) : CheatFolder ("No codes loaded", NULL)
	{
	}
	
	~CheatCodelist ();
	
	bool load (FILE* fp, const char gameid[4], uint32_t headerCRC, bool filter);

	CheatGame* getGame (const char gameid[4], uint32_t headerCRC);
	
private:
	enum TOKEN_TYPE {TOKEN_DATA, TOKEN_TAG_START, TOKEN_TAG_END, TOKEN_TAG_SINGLE};

	std::string nextToken (FILE* fp, TOKEN_TYPE& tokenType);

} ;

#endif // CHEAT_H

