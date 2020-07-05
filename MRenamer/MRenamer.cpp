#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <set>
#include <windows.h>
#include <fstream>

#include "taglib/fileref.h"
#include "taglib/taglib.h"
#include <taglib/tag.h>
#include <map>

using namespace std;

struct ChooseBox {
	bool unroll = false;
	bool rename = false;
	bool both = false;
};

ChooseBox choice;
set<string> artists;
auto path_to_artist_list = R"(C:\Users\User1\Music\Renamed\del\artists.txt)";
//auto path_to_folder = R"(C:\Users\User1\Music\)";
auto new_path_to_folder = R"(C:\Users\User1\Music\Renamed\)";

struct MP3Tag {
	string artist_;
	string title_;
	string album_;
	string year_;
	string comment_;
};

bool IsFounded(unsigned int pos) {
	return pos != string::npos;
}

int GetSize(FILE* file) {
	int loc = ftell(file);
	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, loc, SEEK_SET);
	return size;
}

string GetExtension(const string& filename) {
	int found = filename.find_last_of('.');
	return filename.substr(found + 1);
}

void RememberArtist() {
	fstream artists_file(path_to_artist_list, ios_base::app);
	if (artists_file.is_open()) {
		for (const auto& artist : artists)
			artists_file << artist << endl;
	}
	else {

	}
	artists_file.close();
}

void MakeClean(string& name, const string& flag_sign) {
	auto save_name = name;
	if (IsFounded(name.find(flag_sign))) {
				name.erase(name.begin() + name.find(flag_sign), name.end());
	}
	if (name.length() < 2) {
		save_name.erase(save_name.begin(), save_name.begin()+save_name.find(flag_sign)+1);
		if ((int)*save_name.rbegin() > (int)* flag_sign.begin()) {
			save_name.erase(save_name.end()-1, save_name.end());
		}
		name = save_name;
	}
}

void DeleteSymbolIfExist(string& name, char sym) {
	if (IsFounded(name.find(sym))) {
		name.erase(name.find(sym), 1);
	}
}

void MarkToDelete(string& name, const string& flag_sign, const string& sign_word) {
	if (IsFounded(name.find(flag_sign))) {
		if (IsFounded(name.find(sign_word))) {
			if (name.find(flag_sign) < name.find(sign_word)) {
				name ="del____" +  name ;
			}
		}
	}
}

void RemoveOtherStuff(string& name) {
	string buff = name;
	if (IsFounded(name.find('('))) {
		if (IsFounded(name.find(')'))) {
			name.erase(name.begin() + name.find('('), name.begin() + name.find(')'));
		}
		else
			name.erase(name.begin() + name.find('('), name.end());
	}
	if (IsFounded(name.find('['))) {
		if (IsFounded(name.find('['))) {
			name.erase(name.begin() + name.find('['), name.begin() + name.find(']'));
		}
		else
			name.erase(name.begin() + name.find('['), name.end());
	}
	if (IsFounded(name.find('['))) {
		if (IsFounded(name.find('['))) {
			name.erase(name.begin() + name.find('['), name.begin() + name.find(']'));
		}
		else
			name.erase(name.begin() + name.find('['), name.end());
	}
	if (name.empty()) {
		name = buff;
	}
	DeleteSymbolIfExist(name, ':');
	DeleteSymbolIfExist(name, '\\');
	DeleteSymbolIfExist(name, '*');
	DeleteSymbolIfExist(name, '?');
	DeleteSymbolIfExist(name, '<');
	DeleteSymbolIfExist(name, '>');
	DeleteSymbolIfExist(name, '|');
	std::replace(name.begin(), name.end(), '/', '\&');
	std::replace(name.begin(), name.end(), '_', ' ');
	name = name.substr(0, distance(name.begin(), remove_if(name.begin(), name.end(),
		[](const char& a) {
		return (a == ' ') && (*(&a + 1) == ' ');
	})));
	if (name.empty()) {
		name = buff;
	}
	if (*name.rbegin() == ' ') {
		name.erase(name.end() - 1, name.end());
	}
	if (name.empty()) {
		name = buff;
	}
	if (*name.begin() == ' ') {
		name.erase(name.begin(), name.begin() + 1);
	}
	/* name = name.substr(0, distance(name.begin(), remove_if(name.begin(), name.end(),
															[](const char &a) {
																return (a == '_');
															})));*/
	name.erase(std::remove(name.begin(), name.end(), '_'), name.end());
	name.erase(std::remove(name.begin(), name.end(), '\000'), name.end());
	std::replace(name.begin(), name.end(), ' ', '_');

}

string NameForTAG(string name) {
	std::replace(name.begin(), name.end(), '_', ' ');
	for (int n = 0; n < name.length(); n++)
	{
		if ((n == 0) || (name[n - 1] == ' '))
			name[n] = toupper(name[n]);
	}
	return name;
}

void CheckForBadChars(string& name) {
	auto save_name = name;
	auto bad_signs = {'/', '\\', ':', '*', '?', '"', '\'', '|'};
	for (auto sign : bad_signs)
	{
		if (IsFounded(name.find(sign))) {
			save_name.erase(save_name.begin(), save_name.begin() + save_name.find(sign) + 1);
		}
	}
	if (!save_name.empty()) {
		name = save_name;
	}
}

void CleanName(string& name) {
	transform(name.begin(), name.end(), name.begin(), (int (*)(int)) tolower);
	MarkToDelete(name, "(", "coustic");
	MarkToDelete(name, "(", "ive");
	MarkToDelete(name, "[", "coustic");
	MarkToDelete(name, "[", "ive");
	MakeClean(name, "[");
	MakeClean(name, "(");
	CheckForBadChars(name);
	//cout << "\n in clean  " << name << endl;

	RemoveOtherStuff(name);
}

vector<string> ScanDir(string path) {
	WIN32_FIND_DATA founded_file_data;
	HANDLE handle;
	path = path + '*';
	handle = FindFirstFile(path.c_str(), &founded_file_data);
	_BY_HANDLE_FILE_INFORMATION* f;
	vector<string> files;
	if (handle != INVALID_HANDLE_VALUE) {
		do {
			if (founded_file_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
				GetFileAttributes(path.c_str());
				files.emplace_back(founded_file_data.cFileName);
			}
		} while (FindNextFile(handle, &founded_file_data) != 0);
	}
	return files;
}

void fileRelocation(string path, string dist, string file) {
	string new_path = dist + file;
	string old_path = path + file;
	if (rename(old_path.c_str(), new_path.c_str())) {
		cout << "error renaming " << old_path << " to " << new_path << endl;
	}
}

void UnrollFolder(string path, string dist) {
	WIN32_FIND_DATA founded_file_data;
	HANDLE handle;
	string fun_path = path + '*';
	handle = FindFirstFile(fun_path.c_str(), &founded_file_data);
	_BY_HANDLE_FILE_INFORMATION* f;
	vector<string> files;
	vector<string> folders;
	if (handle != INVALID_HANDLE_VALUE) {
		FindNextFile(handle, &founded_file_data);
		
		while (FindNextFile(handle, &founded_file_data) != 0) {
			if (founded_file_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
				GetFileAttributes(fun_path.c_str());
				string new_path = path + founded_file_data.cFileName + '\\';
				UnrollFolder(new_path, dist);
			}
		}
	}
	handle = FindFirstFile(fun_path.c_str(), &founded_file_data);
	
		if (handle != INVALID_HANDLE_VALUE) {
			do{
				if (founded_file_data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
					GetFileAttributes(fun_path.c_str());
					fileRelocation(path, dist, founded_file_data.cFileName);
				}
			}while (FindNextFile(handle, &founded_file_data) != 0);
		}
	
	return;
}

bool CheckForAnalogue(string& s) {
	auto backup = s;
	s.erase(remove(s.begin(), s.end(), '_'), s.end());
	s.erase(remove(s.begin(), s.end(), ' '), s.end());

	for (auto str : artists) {
		auto another_backup = str;
		str.erase(remove(str.begin(), str.end(), ' '), str.end());
		str.erase(remove(str.begin(), str.end(), '_'), str.end());
		if (s == str) {
			s = another_backup;
			return true;
		}
	}
	s = backup;
	return false;
	/*   vector<string> pieces;
	   vector<string> another_piece
	   while(IsFounded(s.find_first_of('_'))){
		   auto found = s.find_first_of('_');
		   auto piece = s.substr(0, found);
		   another_pieces.push_back(piece);
		   s = s.substr(found+1);
	   }
	   another_pieces.push_back(s);
	   for(auto str: artists){
		   pieces.clear();
		   if(str == backup){
			   continue;
		   }
		   while(IsFounded(str.find_first_of('_'))){
			   auto found = str.find_first_of('_');
			   auto piece = str.substr(0, found);
			   pieces.push_back(piece);
			   str = str.substr(found+1);
		   }
		   pieces.push_back(str);
		   s = "";
		   bool flag = false, flag_another = false;

			   for(auto j : another_pieces){
				   for(auto i: pieces){
				   if(IsFounded(i.find(j)) && !flag){
					   flag_another = true;
					   s += j;
				   }else{
					   if(IsFounded(j.find(i))&& !flag_another){
						   flag = true;
					   }else{
						   flag = false;
						   flag_another = false;
						   s = "";
						   break;
					   }
				   }
			   }
			   if(flag || flag_another){
				   return;
			   }
		   }
		   s = backup;
	   }
	   s = backup;*/
}

string some(vector<string> name1, int& num, bool& is_reverse) {
	is_reverse = false;
	num = 0;
	string maybe_this = name1[0];
	if (CheckForAnalogue(maybe_this)) {
		return maybe_this;
	}

	for (int i = 1; i < name1.size(); i++) {
		maybe_this += "_" + name1[i];
		if (CheckForAnalogue(maybe_this)) {
			num = i;
			return maybe_this;
		}
	}
	is_reverse = true;
	num = name1.size() - 1;
	maybe_this = *name1.rbegin();
	if (CheckForAnalogue(maybe_this)) {
		return maybe_this;
	}

	for (int i = name1.size() - 2; i > 0; i--) {
		maybe_this = name1[i] + "_" + maybe_this;
		if (CheckForAnalogue(maybe_this)) {
			num = i;
			return maybe_this;
		}
	}
	is_reverse = false;
	return name1[0];
}

void CleanAndMore(string& name) {
	CleanName(name);
	cout << "\n after clean  " << name << endl;

	auto pos = name.find('.', name.length() - 5);
	name = name.substr(0, pos);
	replace_if(name.begin(), name.end(), [](char c) {
		return isdigit(c);
	}, ' ');
	transform(name.begin(), name.end(), name.begin(), (int (*)(int)) tolower);
	pos = name.find('-');
	if (IsFounded(pos)) {
		name.erase(pos, 1);
	}
	cout << "\n end clean  " << name << endl;

}

void ReadArtistsNames() {
	fstream artists_file(path_to_artist_list);
		if (artists_file.is_open()) {
			artists_file.seekg(ios_base::beg);
			string s;
			// Считка слов из файла
			for (artists_file >> s; !artists_file.eof(); artists_file >> s)
				artists.insert(s);
		}
		artists_file.close();
}

pair<string, string> TryToFigureOutName(string name) {
	CleanName(name);
	auto pos = name.find('.', name.length() - 5);
	name = name.substr(0, pos);
	replace_if(name.begin(), name.end(), [](char c) {
		return isdigit(c);
	}, ' ');
	transform(name.begin(), name.end(), name.begin(), (int (*)(int)) tolower);
	pos = name.find_first_of('.');
	if (IsFounded(pos)) {
		name = name.substr(pos + 1);
	}

	vector<string> names_pieces;
	string artist_name = name;
	int found;
	bool flag = false;
	while (IsFounded(found = artist_name.find('\226'))) {
		names_pieces.push_back(artist_name.substr(0, found));
		artist_name = artist_name.substr(found + 1);
		flag = true;
	}
	if (flag) {
		names_pieces.push_back(artist_name);
	}
	while (IsFounded(found = artist_name.find('-')) && found != artist_name.length() - 1) {
		names_pieces.push_back(artist_name.substr(0, found));
		artist_name = artist_name.substr(found + 1);
	}
	if (found == artist_name.length() - 1) {
		artist_name = artist_name.substr(0, found);
	}
	if (!flag) {
		names_pieces.push_back(artist_name);
	}
	if (names_pieces.size() == 1) {
		names_pieces.clear();
		while (IsFounded(found = artist_name.find('_'))) {
			names_pieces.push_back(artist_name.substr(0, found));
			artist_name = artist_name.substr(found + 1);
		}
		names_pieces.push_back(artist_name);
	}
	bool is_reverse;
	artist_name = some(names_pieces, found, is_reverse);
	if (is_reverse) {
		name = names_pieces[0];
		for (int i = 1; i < found; ++i) {
			name += "_" + names_pieces[i];
		}
	}
	else {
		if (found + 1 >= names_pieces.size()) {
			found = -1;
			artist_name = "";
		}
		name = names_pieces[found + 1];
		for (int i = found + 2; i < names_pieces.size(); ++i) {
			name += "_" + names_pieces[i];
		}
	}
	auto full_name = name + "-" + artist_name;
	return make_pair(name, artist_name);
}

void Rename(string path_to_folder) {
	auto files = ScanDir(path_to_folder);
	ReadArtistsNames();
	for (const auto& file_name : files) {
		string path_name = (path_to_folder + file_name);
		string new_path;
		string new_name;
		bool flag = false;
		{
			TagLib::FileRef f(path_name.c_str());
			if (!f.isNull()) {
				flag = true;
				TagLib::Tag* tag = f.tag();
				string title = tag->title().toCString();
				string artist = tag->artist().toCString();
				//cout << "Begin " << title  << artist << endl;
				if (!title.empty()) {
					CleanName(title);
				}
				if (!artist.empty()) {
					CleanName(artist);
				}

				tag->setTitle(NameForTAG());
				tag->setArtist(NameForTAG(new_name.second));
				/*cout << "a bit later " << title << artist << endl;

				if (artist.empty() || title.empty()) {
					auto new_name = TryToFigureOutName(file_name);
					string full_new_name;
					if (title.empty()) {
						title = file_name;
						cout << "NOT SO BAD " << title << endl;

						CleanAndMore(title);
						full_new_name = title + "-" + artist + "." + GetExtension(file_name);
						new_name.first = title;
					}
					else {
						cout << "BAD " << path_name << endl;
						CleanName(new_name.first);
						full_new_name = new_name.first + "-" + new_name.second + "." + GetExtension(file_name);
					}

					cout << "Is it Right Name?  " << full_new_name << " Enter y/n , please \n";
					char r = 'y';
					cin >> r;
					if (r != 'n') {
						tag->setTitle(NameForTAG(new_name.first));
						tag->setArtist(NameForTAG(new_name.second));
						f.save();
						string new_path_ = new_path_to_folder + full_new_name;
						title = new_name.first;
						artist = new_name.second;

						//if (rename(path_name.c_str(), new_path_.c_str())) {
						 //  cout << "error " << full_new_name << endl;
						 //  continue;
					  // }
					}
					if (r == 'n') {
						title = file_name;

						cout << "\n Inter artist name  " << file_name << endl;
						cin >> artist;

						auto full_new_name = title + "-" + artist + "." + GetExtension(file_name);

						cout << "Is it Right Name?  " << full_new_name << " Enter y/n , please \n";
						char r = 'y';
						cin >> r;
						if (r != 'n') {
							tag->setTitle(NameForTAG(new_name.first));
							tag->setArtist(NameForTAG(new_name.second));
							f.save();
						}
					}
					//cout << "All Right!" << endl;

				}*/
				CheckForAnalogue(artist);
				artists.insert(artist);
				new_name = title + "-" + artist + "." + GetExtension(file_name);
				//cout << "new name  - \"" << new_name << "\"" << endl;
				new_path = new_path_to_folder + new_name;
			}
		}
		if (flag) {
			auto err_no = rename(path_name.c_str(), new_path.c_str());
			if (!err_no) {
				//cout << "All Right!" << endl;
			}
			else {
				new_name = new_name + "_copy_";
				string new_path = path_to_folder + new_name;
				if (rename(path_name.c_str(), new_path.c_str())) {
					cout << "error renaming " << path_name << " to " << new_path << endl;
					continue;
				}
				cout << "All Right!" << endl;
			}
		}
	}
	RememberArtist();
}


int main() {
	//cin TODO
	cout << "You wanna to unroll folder (u) or rename music folder (r) or both (b)?  Enter char , please \n";
	char right;
	cin >> right;
	switch(right) {
	case 'u': {
		choice.unroll = true;
		cout << "Enter path to folder, please (like C:\\Users\\User1\\Music\\Incubus\\) \n";
		string path, dist;
		cin >> path;
		cout << "and distanation folder, please (like C:\\Users\\User1\\Music\\Incubus\\) \n";
		cin >> dist;
		UnrollFolder(path, dist);
		break;
	}
		case (int)'b': {
			choice.both = true;
			cout << "Enter path to folder, please (like C:\\Users\\User1\\Music\\Incubus\\) \n";
			string path, dist;
			cin >> path;
			cout << "and distanation folder, please (like C:\\Users\\User1\\Music\\Incubus\\) \n";
			cin >> dist;
			UnrollFolder(path, dist);
			Rename(path);
			break; 
		}
		case (int)'r': {
			choice.rename = true;
			string path_to_folder;
			cout << "Enter path to folder, please (like C:\\Users\\User1\\Music\\) \n";
			cin >> path_to_folder;
			Rename(path_to_folder);
			break; 
		}
		default: {
			cout << "Put your shit together and try again! \n"; 
			exit(1);
		}
	}
	if (choice.unroll) {
		

	}
	if (choice.rename) {
		
		/*auto files = ScanDir(path_to_folder);
		ReadArtistsNames();
		for (const auto& file_name : files) {
			string path_name = (path_to_folder + file_name);
			string new_path;
			string new_name;
			bool flag = false;
			{
				TagLib::FileRef f(path_name.c_str());
				if (!f.isNull()) {
					flag = true;
					TagLib::Tag* tag = f.tag();
					string title = tag->title().toCString();
					string artist = tag->artist().toCString();
					//cout << "Begin " << title  << artist << endl;
					if (!title.empty()) {
						CleanName(title);
					}
					if (!artist.empty()) {
						CleanName(artist);
					}
					/*cout << "a bit later " << title << artist << endl;

					if (artist.empty() || title.empty()) {
						auto new_name = TryToFigureOutName(file_name);
						string full_new_name;
						if (title.empty()) {
							title = file_name;
							cout << "NOT SO BAD " << title << endl;

							CleanAndMore(title);
							full_new_name = title + "-" + artist + "." + GetExtension(file_name);
							new_name.first = title;
						}
						else {
							cout << "BAD " << path_name << endl;
							CleanName(new_name.first);
							full_new_name = new_name.first + "-" + new_name.second + "." + GetExtension(file_name);
						}

						cout << "Is it Right Name?  " << full_new_name << " Enter y/n , please \n";
						char r = 'y';
						cin >> r;
						if (r != 'n') {			
							tag->setTitle(NameForTAG(new_name.first));
							tag->setArtist(NameForTAG(new_name.second));
							f.save();
							string new_path_ = new_path_to_folder + full_new_name;
							title = new_name.first;
							artist = new_name.second;

							//if (rename(path_name.c_str(), new_path_.c_str())) {
							 //  cout << "error " << full_new_name << endl;
							 //  continue;
						  // }
						}
						if (r == 'n') {
							title = file_name;

							cout << "\n Inter artist name  " << file_name << endl;
							cin >> artist;

							auto full_new_name = title + "-" + artist + "." + GetExtension(file_name);

							cout << "Is it Right Name?  " << full_new_name << " Enter y/n , please \n";
							char r = 'y';
							cin >> r;
							if (r != 'n') {
								tag->setTitle(NameForTAG(new_name.first));
								tag->setArtist(NameForTAG(new_name.second));
								f.save();
							}
						}
						//cout << "All Right!" << endl;

					}
					CheckForAnalogue(artist);
					artists.insert(artist);
					new_name = title + "-" + artist + "." + GetExtension(file_name);
					//cout << "new name  - \"" << new_name << "\"" << endl;
					new_path = new_path_to_folder + new_name;
				}
			}
			if (flag) {
				auto err_no = rename(path_name.c_str(), new_path.c_str());
				if (!err_no) {
					//cout << "All Right!" << endl;
				}
				else {
					new_name = new_name + "_copy_";
					string new_path = path_to_folder + new_name;
					if (rename(path_name.c_str(), new_path.c_str())) {
						cout << "error renaming " << path_name << " to " << new_path << endl;
						continue;
					}
					cout << "All Right!" << endl;
				}
			}
		}
		RememberArtist();*/
	}
	return 0;
}
