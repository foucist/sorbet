#include "gtest/gtest.h"
#include <cxxopts.hpp>
// has to go first as it violates requirements

#include "common/common.h"
#include "test/helpers/lsp.h"

namespace sorbet::test {
using namespace std;

template <typename T = DynamicRegistrationOption>
std::unique_ptr<T> makeDynamicRegistrationOption(bool dynamicRegistration) {
    auto option = std::make_unique<T>();
    option->dynamicRegistration = dynamicRegistration;
    return option;
};

/** Constructs a vector with all enum values from MIN to MAX. Assumes a contiguous enum and properly chosen min/max
 * values. Our serialization/deserialization code will throw if we pick an improper value. */
template <typename T, T MAX, T MIN> std::vector<T> getAllEnumKinds() {
    std::vector<T> symbols;
    for (int i = (int)MIN; i <= (int)MAX; i++) {
        symbols.push_back((T)i);
    }
    return symbols;
};

unique_ptr<SymbolConfiguration> makeSymbolConfiguration() {
    auto config = make_unique<SymbolConfiguration>();
    config->dynamicRegistration = true;
    auto symbolOptions = make_unique<SymbolKindOptions>();
    symbolOptions->valueSet = getAllEnumKinds<SymbolKind, SymbolKind::File, SymbolKind::TypeParameter>();
    config->symbolKind = std::move(symbolOptions);
    return config;
}

unique_ptr<WorkspaceClientCapabilities> makeWorkspaceClientCapabilities() {
    auto capabilities = make_unique<WorkspaceClientCapabilities>();
    // The following client config options were cargo-culted from existing tests.
    // TODO(jvilk): Prune these down to only the ones we care about.
    capabilities->applyEdit = true;
    auto workspaceEdit = make_unique<WorkspaceEditCapabilities>();
    workspaceEdit->documentChanges = true;
    capabilities->workspaceEdit = unique_ptr<WorkspaceEditCapabilities>(std::move(workspaceEdit));
    capabilities->didChangeConfiguration = makeDynamicRegistrationOption(true);
    capabilities->didChangeWatchedFiles = makeDynamicRegistrationOption(true);
    capabilities->symbol = makeSymbolConfiguration();
    capabilities->executeCommand = makeDynamicRegistrationOption(true);
    capabilities->configuration = true;
    capabilities->workspaceFolders = true;
    return capabilities;
}

unique_ptr<TextDocumentClientCapabilities> makeTextDocumentClientCapabilities() {
    auto capabilities = make_unique<TextDocumentClientCapabilities>();

    auto publishDiagnostics = make_unique<PublishDiagnosticsCapabilities>();
    publishDiagnostics->relatedInformation = true;
    capabilities->publishDiagnostics = std::move(publishDiagnostics);

    auto synchronization = makeDynamicRegistrationOption<SynchronizationCapabilities>(true);
    synchronization->willSave = true;
    synchronization->willSaveWaitUntil = true;
    synchronization->didSave = true;
    capabilities->synchronization = std::move(synchronization);

    auto completion = makeDynamicRegistrationOption<CompletionCapabilities>(true);
    completion->contextSupport = true;
    auto completionItem = make_unique<CompletionItemCapabilities>();
    completionItem->snippetSupport = true;
    completionItem->commitCharactersSupport = true;
    completionItem->documentationFormat = {MarkupKind::Markdown, MarkupKind::Plaintext};
    completion->completionItem = std::move(completionItem);
    auto completionItemKind = make_unique<CompletionItemKindCapabilities>();
    completionItemKind->valueSet =
        getAllEnumKinds<CompletionItemKind, CompletionItemKind::Text, CompletionItemKind::TypeParameter>();
    completion->completionItemKind = std::move(completionItemKind);
    capabilities->completion = std::move(completion);

    auto hover = makeDynamicRegistrationOption<HoverCapabilities>(true);
    hover->contentFormat = {MarkupKind::Markdown, MarkupKind::Plaintext};
    capabilities->hover = std::move(hover);

    auto signatureHelp = makeDynamicRegistrationOption<SignatureHelpCapabilities>(true);
    auto signatureInformation = make_unique<SignatureInformationCapabilities>();
    signatureInformation->documentationFormat = {MarkupKind::Markdown, MarkupKind::Plaintext};
    signatureHelp->signatureInformation = std::move(signatureInformation);
    capabilities->signatureHelp = std::move(signatureHelp);

    auto documentSymbol = makeDynamicRegistrationOption<DocumentSymbolCapabilities>(true);
    auto symbolKind = make_unique<SymbolKindOptions>();
    symbolKind->valueSet = getAllEnumKinds<SymbolKind, SymbolKind::File, SymbolKind::TypeParameter>();
    documentSymbol->symbolKind = move(symbolKind);
    capabilities->documentSymbol = move(documentSymbol);

    capabilities->definition = makeDynamicRegistrationOption(true);
    capabilities->references = makeDynamicRegistrationOption(true);
    capabilities->documentHighlight = makeDynamicRegistrationOption(true);
    capabilities->codeAction = makeDynamicRegistrationOption<CodeActionCapabilities>(true);
    capabilities->codeLens = makeDynamicRegistrationOption(true);
    capabilities->formatting = makeDynamicRegistrationOption(true);
    capabilities->rangeFormatting = makeDynamicRegistrationOption(true);
    capabilities->onTypeFormatting = makeDynamicRegistrationOption(true);
    capabilities->rename = makeDynamicRegistrationOption<RenameCapabilities>(true);
    capabilities->documentLink = makeDynamicRegistrationOption(true);
    capabilities->typeDefinition = makeDynamicRegistrationOption(true);
    capabilities->implementation = makeDynamicRegistrationOption(true);
    capabilities->colorProvider = makeDynamicRegistrationOption(true);

    return capabilities;
}

unique_ptr<JSONBaseType> makeInitializeParams(string rootPath, string rootUri) {
    auto initializeParams = make_unique<InitializeParams>(12345, rootPath, rootUri, make_unique<ClientCapabilities>());
    initializeParams->capabilities->workspace = makeWorkspaceClientCapabilities();
    initializeParams->capabilities->textDocument = makeTextDocumentClientCapabilities();
    initializeParams->trace = TraceKind::Off;

    auto workspaceFolder = make_unique<WorkspaceFolder>(rootUri, "pay-server");
    vector<unique_ptr<WorkspaceFolder>> workspaceFolders;
    workspaceFolders.push_back(move(workspaceFolder));
    initializeParams->workspaceFolders =
        make_optional<variant<JSONNullObject, vector<unique_ptr<WorkspaceFolder>>>>(move(workspaceFolders));
    return initializeParams;
}

unique_ptr<LSPMessage> makeRequestMessage(rapidjson::MemoryPoolAllocator<> &alloc, string method, int id,
                                          const JSONBaseType &params) {
    auto initialize = make_unique<RequestMessage>("2.0", id, method);
    initialize->params = params.toJSONValue(alloc);
    return make_unique<LSPMessage>(move(initialize));
}

unique_ptr<LSPMessage> makeDefinitionRequest(rapidjson::MemoryPoolAllocator<> &alloc, int id, std::string_view uri,
                                             int line, int character) {
    unique_ptr<JSONBaseType> textDocumentPositionParams = make_unique<TextDocumentPositionParams>(
        make_unique<TextDocumentIdentifier>(string(uri)), make_unique<Position>(line, character));
    return makeRequestMessage(alloc, "textDocument/definition", id, *textDocumentPositionParams);
}

/** Checks that we are properly advertising Sorbet LSP's capabilities to clients. */
void checkServerCapabilities(const ServerCapabilities &capabilities) {
    // Properties checked in the same order they are described in the LSP spec.
    EXPECT_TRUE(capabilities.textDocumentSync.has_value());
    auto &textDocumentSync = *(capabilities.textDocumentSync);
    auto textDocumentSyncValue = get_if<TextDocumentSyncKind>(&textDocumentSync);
    EXPECT_NE(nullptr, textDocumentSyncValue);
    if (textDocumentSyncValue != nullptr) {
        EXPECT_EQ(TextDocumentSyncKind::Full, *(textDocumentSyncValue));
    }

    EXPECT_TRUE(capabilities.hoverProvider.value_or(false));

    EXPECT_TRUE(capabilities.completionProvider.has_value());
    if (capabilities.completionProvider.has_value()) {
        auto &completionProvider = *(capabilities.completionProvider);
        auto triggerCharacters = completionProvider->triggerCharacters.value_or(vector<string>({}));
        EXPECT_EQ(1, triggerCharacters.size());
        if (triggerCharacters.size() == 1) {
            EXPECT_EQ(".", triggerCharacters.at(0));
        }
    }

    EXPECT_TRUE(capabilities.signatureHelpProvider.has_value());
    if (capabilities.signatureHelpProvider.has_value()) {
        auto &sigHelpProvider = *(capabilities.signatureHelpProvider);
        auto sigHelpTriggerChars = sigHelpProvider->triggerCharacters.value_or(vector<string>({}));
        EXPECT_EQ(2, sigHelpTriggerChars.size());
        UnorderedSet<string> sigHelpTriggerSet(sigHelpTriggerChars.begin(), sigHelpTriggerChars.end());
        EXPECT_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find("("));
        EXPECT_NE(sigHelpTriggerSet.end(), sigHelpTriggerSet.find(","));
    }

    // We don't support all possible features. Make sure we don't make any false claims.
    EXPECT_TRUE(capabilities.definitionProvider.value_or(false));
    EXPECT_FALSE(capabilities.typeDefinitionProvider.has_value());
    EXPECT_FALSE(capabilities.implementationProvider.has_value());
    EXPECT_TRUE(capabilities.referencesProvider.value_or(false));
    EXPECT_FALSE(capabilities.documentHighlightProvider.has_value());
    EXPECT_TRUE(capabilities.documentSymbolProvider.value_or(false));
    EXPECT_TRUE(capabilities.workspaceSymbolProvider.value_or(false));
    EXPECT_FALSE(capabilities.codeActionProvider.has_value());
    EXPECT_FALSE(capabilities.codeLensProvider.has_value());
    EXPECT_FALSE(capabilities.documentFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.documentRangeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.documentRangeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.documentOnTypeFormattingProvider.has_value());
    EXPECT_FALSE(capabilities.renameProvider.has_value());
    EXPECT_FALSE(capabilities.documentLinkProvider.has_value());
    EXPECT_FALSE(capabilities.colorProvider.has_value());
    EXPECT_FALSE(capabilities.foldingRangeProvider.has_value());
    EXPECT_FALSE(capabilities.executeCommandProvider.has_value());
    EXPECT_FALSE(capabilities.workspace.has_value());
}

void failWithUnexpectedLSPResponse(const LSPMessage &item) {
    if (item.isResponse()) {
        FAIL() << fmt::format("Expected a notification, but received the following response message instead: {}",
                              item.asResponse().toJSON());
    } else if (item.isNotification()) {
        FAIL() << fmt::format("Expected a response message, but received the following notification instead: {}",
                              item.asNotification().toJSON());
    } else {
        // Received something else! This can *only* happen if our test logic is buggy. Any invalid LSP responses
        // are rejected before they reach this point.
        FAIL() << "Received unexpected response message type; this should never happen.";
    }
}

bool assertResponseMessage(int expectedId, const LSPMessage &response) {
    if (!response.isResponse()) {
        failWithUnexpectedLSPResponse(response);
        return false;
    }

    auto &respMsg = response.asResponse();
    auto idIntPtr = get_if<int>(&respMsg.id);
    EXPECT_NE(nullptr, idIntPtr) << "Response message lacks an integer ID field.";
    if (idIntPtr != nullptr) {
        EXPECT_EQ(expectedId, *idIntPtr) << "Response message's ID does not match expected value.";
    }
    return true;
}

bool assertNotificationMessage(const string &expectedMethod, const LSPMessage &response) {
    if (!response.isNotification()) {
        failWithUnexpectedLSPResponse(response);
        return false;
    }
    EXPECT_EQ(expectedMethod, response.method()) << "Unexpected method on notification message.";
    return true;
}

optional<unique_ptr<PublishDiagnosticsParams>> getPublishDiagnosticParams(rapidjson::MemoryPoolAllocator<> &alloc,
                                                                          const NotificationMessage &notifMsg) {
    if (!notifMsg.params.has_value()) {
        ADD_FAILURE() << "textDocument/publishDiagnostics message is missing parameters.";
        return nullopt;
    }
    auto &params = *notifMsg.params;
    auto paramsValuePtr = get_if<unique_ptr<rapidjson::Value>>(&params);
    if (!paramsValuePtr) {
        // It's an array rather than a single object.
        ADD_FAILURE() << "textDocument/publishDiagnostics message unexpectedly had array in `params` field";
        return nullopt;
    }
    auto &paramsValue = *paramsValuePtr;
    return PublishDiagnosticsParams::fromJSONValue(alloc, *paramsValue.get(), "NotificationMessage.params");
}

void initializeLSP(string_view rootPath, string_view rootUri, LSPWrapper &lspWrapper, int &nextId) {
    // Reset next id.
    nextId = 0;

    // Send 'initialize' message.
    {
        unique_ptr<JSONBaseType> initializeParams = makeInitializeParams(string(rootPath), string(rootUri));
        auto responses = lspWrapper.getLSPResponsesFor(
            *makeRequestMessage(lspWrapper.alloc, "initialize", nextId++, *initializeParams));

        // Should just have an 'initialize' response.
        ASSERT_EQ(1, responses.size());

        if (assertResponseMessage(0, *responses.at(0))) {
            auto &respMsg = responses.at(0)->asResponse();
            ASSERT_FALSE(respMsg.error.has_value());
            ASSERT_TRUE(respMsg.result.has_value());

            auto &result = *respMsg.result;
            auto initializeResult =
                InitializeResult::fromJSONValue(lspWrapper.alloc, *result.get(), "ResponseMessage.result");
            checkServerCapabilities(*initializeResult->capabilities);
        }
    }

    // Complete initialization handshake with an 'initialized' message.
    {
        rapidjson::Value emptyObject(rapidjson::kObjectType);
        auto initialized = make_unique<NotificationMessage>("2.0", "initialized");
        initialized->params = make_unique<rapidjson::Value>(rapidjson::kObjectType);
        auto initializedResponses = lspWrapper.getLSPResponsesFor(LSPMessage(move(initialized)));
        EXPECT_EQ(0, initializedResponses.size()) << "Should not receive any response to 'initialized' message.";
    }
}

} // namespace sorbet::test