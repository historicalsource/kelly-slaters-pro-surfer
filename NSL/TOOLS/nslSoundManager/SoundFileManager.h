#ifndef SOUNDFILENMANAGER_H
#define SOUNDFILENMANAGER_H

// SoundFileManager.h : header file
//

// Treyarch LLC
// Andrew Olson 10-24-01


#include <afxtempl.h>

// I know I should separate the definition from the implementation, but 
//   I want to get this done quick, and it's short enough

// there is clearly a better way to go about this than creating these classes
//   unfortunately, I don't know how to do it

class Sound
{
protected:
	bool m_enabled;
	CString m_name;		// name (no extension)
	CString m_type;		// SFX, ambient, music, voice
	CString m_location;	// CD, SPU
//	CString m_vol;		// 0-100
	CString m_loop;		// "loop" or ""

public:
	Sound() : m_enabled(true) {}
	Sound(CString str) : m_enabled(true) { ReadFromString(str); }
	void SetName(CString name) { m_name = name; }
	void SetType(CString type) { m_type = type; }
	void SetLocation(CString location) { m_location = location; }
//	void SetVolume(CString vol) { m_vol = vol; }

	CString GetName() { return m_name; }
	CString GetType() { return m_type; }
	CString GetLocation() { return m_location; }
//	CString GetVol() { return m_vol; }

	bool IsEnabled() { return m_enabled; }
	void Enable() { m_enabled = true; }
	void Disable() { m_enabled = false; }

	bool IsLooped() { return !(m_loop==""); }
	void SetLoop() { m_loop = "loop"; }
	void ClearLoop() { m_loop = ""; }

	void ReadFromString(CString str) {
		str.TrimLeft();
		str += " "; // add a space to the end so we can separate parameters by spaces (and ensure an end condition)
		if(str.GetAt(0)==';') m_enabled = false;
		str.TrimLeft(" ;");
		m_name = str.Left(str.Find(" "));
		str = str.Mid(str.Find(" ")+1);
		while(str != "") {
			// types
			str.TrimLeft();
			CString attribute = str.Left(str.Find(" "));
			if(attribute.CompareNoCase("sfx")==0) { m_type = attribute; }
			else if(attribute.CompareNoCase("ambient")==0) { m_type = attribute; }
			else if(attribute.CompareNoCase("music")==0) { m_type = attribute; }
			else if(attribute.CompareNoCase("voice")==0) { m_type = attribute; }
			// location
			else if(attribute.CompareNoCase("cd")==0) { m_location = attribute; }
			else if(attribute.CompareNoCase("spu")==0) { m_location = attribute; }
			// loop
			else if(attribute.CompareNoCase("loop")==0) { m_loop = attribute; }
/*			// volume
			// doesn't quite work, so forget about it
			else if(attribute.CompareNoCase("voll")==0 || attribute.CompareNoCase("volr")==0) {
				str = str.Mid(str.Find(" ")+1);
				m_vol = str;	// note we're not supporting different volumes/channel (regardless of whether NSL does)
			}
*/			else if(attribute == "") break;
			else MessageBox(NULL,CString("Unknown attribute ")+attribute, "Warning", MB_OK);
			str = str.Mid(str.Find(" ")+1);
		}
	}
	void PrintFileLine(FILE* file) {
		fprintf(file,"\n%s%s",(m_enabled?"":";"),(LPCTSTR)m_name);
		if(m_type != "")		fprintf(file," %s",(LPCTSTR)m_type);
		if(m_location != "")	fprintf(file," %s",(LPCTSTR)m_location);
//		if(m_vol != "")			fprintf(file," voll %s volr %s",(LPCTSTR)m_vol,(LPCTSTR)m_vol);
		if(m_loop != "")		fprintf(file," %s",(LPCTSTR)m_loop);
	}
};



class SoundFile		// a level, or COMMON
{
protected:
	CString m_name;
	CList<Sound,Sound&> m_sounds;

public:
	SoundFile() {};
	SoundFile(CString name) : m_name(name) {}
	SoundFile(SoundFile& sf) : m_name(sf.m_name) { m_sounds.AddTail(&(sf.m_sounds)); }
	void SetName(CString name) { m_name = name; }
	CString GetName() { return m_name; }
	Sound& AddSound(CString name) { return m_sounds.GetAt(m_sounds.AddTail( Sound(name) ) ); }
	Sound& GetSound(int n) {
		return m_sounds.GetAt(m_sounds.FindIndex(n));
	}
	Sound& GetSound(CString name) {
		POSITION pos = m_sounds.GetHeadPosition();
		for(int i=0;i<m_sounds.GetCount();i++) {
			if(m_sounds.GetAt(pos).GetName()==name) return m_sounds.GetAt(pos);
			m_sounds.GetNext(pos);
		}
		MessageBox(NULL,"couldn't find sound "+name,"Error",MB_OK);
		return m_sounds.GetAt(0); // just return something
	}
	void RemoveSound(CString name) {
		POSITION pos = m_sounds.GetHeadPosition();
		for(int i=0;i<m_sounds.GetCount();i++) {
			if(m_sounds.GetAt(pos).GetName()==name) {
				m_sounds.RemoveAt(pos);
				return;
			}
			m_sounds.GetNext(pos);
		}
		MessageBox(NULL,"couldn't remove sound "+name,"Error",MB_OK);
	}
	SoundFile ReadSndFile(CString root_dir) {
		CString buf;
		CStdioFile file;
		SoundFile common("COMMON");
		if( !file.Open( (LPCTSTR)(root_dir+"\\"+m_name+".SND"), CFile::modeRead | CFile::typeText ) ) {
			return common;
		}
		while(file.ReadString(buf)) {
			if(buf==";"+m_name)
				continue;
			else if(buf==";COMMON") {
				while(file.ReadString(buf)) {
					common.AddSound(buf);
				}
			}
			else
				AddSound(buf);
		}
		file.Close();
		return common;
	}
	void WriteSndFile(FILE* file) {
		fprintf(file,";%s",(LPCTSTR)m_name);
		POSITION pos = m_sounds.GetHeadPosition();
		Sound sound;
		for(int i=0;i<m_sounds.GetCount();i++) {
			sound = m_sounds.GetAt(pos);
			sound.PrintFileLine(file);
			m_sounds.GetNext(pos);
		}
		fprintf(file,"\n");
	}
	SoundFile& operator=(SoundFile& sf) {
		m_name = sf.m_name;
		m_sounds.RemoveAll();
		m_sounds.AddHead(&(sf.m_sounds));
		return sf;
	}
	int GetNumSounds() {
		return m_sounds.GetCount();
	}
};


/////////////////////////////////////////////////////////////////////////////
// SoundFileManager

class SoundFileManager
{
protected:
	int m_current;
	SoundFile m_common;					// common sounds always loaded
	SoundFile m_empty;					// placeholder, never gets sounds
	CList<SoundFile,SoundFile&> m_levels;	// level-specific sounds

public:
	SoundFileManager() {
		m_current = -1; // default view is of common
		m_common.SetName("COMMON");
		m_empty.SetName("LEVELS");
	}
	SoundFile& AddLevel(CString level) {
		return m_levels.GetAt(m_levels.AddTail( SoundFile(level) ));
	}
	SoundFile& GetLevel(int n) {
		return m_levels.GetAt(m_levels.FindIndex(n));
	}
	SoundFile& GetLevel(CString level) {
		if(level.CompareNoCase(m_common.GetName())==0) return m_common;
		if(level.CompareNoCase(m_empty.GetName())==0) return m_empty;
		POSITION pos = m_levels.GetHeadPosition();
		for(int i=0;i<m_levels.GetCount();i++) {
			if(m_levels.GetAt(pos).GetName()==level) return m_levels.GetAt(pos);
			m_levels.GetNext(pos);
		}
		MessageBox(NULL,"couldn't find level "+level,"Error",MB_OK);
		return GetCurrentLevel();
	}
	void SetCurrentLevel(CString name) {
		if(name.CompareNoCase(m_common.GetName())==0) m_current= -1;
		if(name.CompareNoCase(m_empty.GetName())==0) m_current= -2;
		else {
			POSITION pos = m_levels.GetHeadPosition();
			for(int i=0;i<m_levels.GetCount();i++) {
				if(m_levels.GetAt(pos).GetName()==name) {
					m_current = i;
					return;
				}
				m_levels.GetNext(pos);
			}
		}
	}
	SoundFile& GetCurrentLevel() {
		if(m_current == -1) return m_common;
		if(m_current == -2) return m_empty;

		return m_levels.GetAt(m_levels.FindIndex(m_current));
	}
	int GetNumLevels() {
		return m_levels.GetCount();
	}
};

/////////////////////////////////////////////////////////////////////////////

#endif