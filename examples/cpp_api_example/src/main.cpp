#include <fstream>
#include <iostream>

#include "SourcetrailDBWriter.h"

int main(int argc, const char *argv[])
{
	sourcetrail::SourcetrailDBWriter dbWriter;

	std::cout << "SourcetrailDB Poetry Indexer" << std::endl;
	std::cout << std::endl;
	std::cout << "SourcetrailDB version: " << dbWriter.getVersionString() << std::endl;
	std::cout << "Supported database version: " << dbWriter.getSupportedDatabaseVersion() << std::endl;
	std::cout << std::endl;

	if (argc != 4)
	{
		std::cout << "usage: poetry_indexer <database_path> <database_version> <source_path>";
	}

	std::string dbPath = argv[1];
	int dbVersion = std::stoi(argv[2]);
	std::string sourcePath = argv[3];

	if (dbVersion != dbWriter.getSupportedDatabaseVersion())
	{
		std::cout << "error: binary only supports database version: " << dbWriter.getSupportedDatabaseVersion()
			<< ". Requested version: " << dbVersion << std::endl;
		return 1;
	}

	// open database by passing .srctrldb or .srctrldb_tmp path
	if (!dbWriter.open(dbPath))
	{
		std::cout << "error: " << dbWriter.getLastError() << std::endl;
		return 1;
	}

	// start recording with faster speed
	if (!dbWriter.beginTransaction())
	{
		std::cout << "error: " << dbWriter.getLastError() << std::endl;
		return 1;
	}

	// record source file by passing it's absolute path
	int fileId = dbWriter.recordFile(sourcePath);
	dbWriter.recordFileLanguage(fileId, "cpp"); // record file language for syntax highlighting


	// record comment
	dbWriter.recordCommentLocation({ fileId, 2, 1, 6, 3 });


	// record namespace "api"
	sourcetrail::NameHierarchy namespaceName { "::", { { "", "api", "" } } };
	int namespaceId = dbWriter.recordSymbol(namespaceName);
	dbWriter.recordSymbolDefinitionKind(namespaceId, sourcetrail::DEFINITION_EXPLICIT);
	dbWriter.recordSymbolKind(namespaceId, sourcetrail::SYMBOL_NAMESPACE);
	dbWriter.recordSymbolLocation(namespaceId, { fileId, 8, 11, 8, 13 });
	dbWriter.recordSymbolScopeLocation(namespaceId, { fileId, 8, 1, 24, 1 });


	// record class "MyType"
	sourcetrail::NameHierarchy className = namespaceName;
	className.nameElements.push_back( { "", "MyType", "" } );
	int classId = dbWriter.recordSymbol(className);
	dbWriter.recordSymbolDefinitionKind(classId, sourcetrail::DEFINITION_EXPLICIT);
	dbWriter.recordSymbolKind(classId, sourcetrail::SYMBOL_CLASS);
	dbWriter.recordSymbolLocation(classId, { fileId, 11, 7, 11, 12 });
	dbWriter.recordSymbolScopeLocation(classId, { fileId, 11, 1, 22, 1 }); // gets highlight when active


	// record inheritance reference to "BaseType"
	int baseId = dbWriter.recordSymbol( { "::", { { "", "BaseType", "" } } } );
	int inheritanceId = dbWriter.recordReference(baseId, classId, sourcetrail::REFERENCE_INHERITANCE);
	dbWriter.recordReferenceLocation(inheritanceId, { fileId, 12, 14, 12, 21 });


	// add child method "void my_method() const"
	sourcetrail::NameHierarchy methodName = className;
	methodName.nameElements.push_back( { "void", "my_method", "() const" } );
	int methodId = dbWriter.recordSymbol(methodName);
	dbWriter.recordSymbolDefinitionKind(methodId, sourcetrail::DEFINITION_EXPLICIT);
	dbWriter.recordSymbolKind(methodId, sourcetrail::SYMBOL_METHOD);
	dbWriter.recordSymbolLocation(methodId, { fileId, 15, 10, 15, 18 });
	dbWriter.recordSymbolScopeLocation(methodId, { fileId, 15, 5, 21, 5 }); // gets highlight when active
	dbWriter.recordSymbolSignatureLocation(methodId, { fileId, 15, 5, 15, 45 }); // used in tooltip


	// record parameter type "bool"
	int typeId = dbWriter.recordSymbol({ "::", { { "", "bool", "" } } });
	int typeuseId = dbWriter.recordReference(methodId, typeId, sourcetrail::REFERENCE_TYPE_USAGE);
	dbWriter.recordReferenceLocation(typeuseId, { fileId, 15, 20, 15, 23 });


	// record parameter "do_send_signal"
	int localId = dbWriter.recordLocalSymbol("do_send_signal");
	dbWriter.recordLocalSymbolLocation(localId, { fileId, 15, 25, 15, 38 });
	dbWriter.recordLocalSymbolLocation(localId, { fileId, 17, 13, 17, 26 });


	// record function call reference to "send_signal()"
	int funcId = dbWriter.recordSymbol({ "::", { { "", "Client", "" }, { "", "send_signal", "()" } } });
	dbWriter.recordSymbolKind(funcId, sourcetrail::SYMBOL_FUNCTION);
	int callId = dbWriter.recordReference(methodId, funcId, sourcetrail::REFERENCE_CALL);
	dbWriter.recordReferenceLocation(callId, { fileId, 19, 13, 19, 33 });


	// record error
	dbWriter.recordError("Really? You missed that \";\" again?", false, { fileId, 22, 1, 22, 1 });


	// end recording
	if (!dbWriter.commitTransaction())
	{
		std::cout << "error: " << dbWriter.getLastError() << std::endl;
		return 1;
	}

	// check for errors before finishing
	if (dbWriter.getLastError().size())
	{
		std::cout << "error: " << dbWriter.getLastError() << std::endl;
		return 1;
	}

	return 0;
}