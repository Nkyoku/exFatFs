#include "minifs_filesystem.h"
#include "minifs_dirhandle.h"
#include "minifs_filehandle.h"


namespace mfs{
	// IMiniFSFileSystemをインスタンス化
	IMiniFSFileSystem NullFs;



	// ディレクトリを開く
	DirHandle* IMiniFSFileSystem::openDir(const fschar_t *path){
		DirHandle *pdirhandle = new DirHandle();
		if (pdirhandle->open(*this, path) != RES_SUCCEEDED){
			delete pdirhandle;
			pdirhandle = nullptr;
		}
		return pdirhandle;
	}

	// ファイルを開く
	FileHandle* IMiniFSFileSystem::openFile(const fschar_t *path, uint32_t option){
		FileHandle *pfilehandle = new FileHandle();
		if (pfilehandle->open(*this, path, option) != RES_SUCCEEDED){
			delete pfilehandle;
			pfilehandle = nullptr;
		}
		return pfilehandle;
	}



}
