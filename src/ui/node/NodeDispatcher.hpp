#pragma once
#include "base/Qv2rayBase.hpp"
#include "base/models/QvComplexConfigModels.hpp"

#include <nodes/FlowScene>

class NodeDispatcher
    : public QObject
    , public std::enable_shared_from_this<NodeDispatcher>
{
    Q_OBJECT
  public:
    explicit NodeDispatcher(QObject *parent = nullptr);
    ~NodeDispatcher();
    void InitializeScenes(QtNodes::FlowScene *rule, QtNodes::FlowScene *chain)
    {
        ruleScene = rule;
        chainScene = chain;
        connect(ruleScene, &QtNodes::FlowScene::nodeDeleted, this, &NodeDispatcher::OnNodeDeleted);
        connect(chainScene, &QtNodes::FlowScene::nodeDeleted, this, &NodeDispatcher::OnNodeDeleted);
    }

  public:
    CONFIGROOT GetFullConfig() const;
    void LoadFullConfig(const CONFIGROOT &);
    [[nodiscard]] QString CreateInbound(INBOUND);
    [[nodiscard]] QString CreateOutbound(OutboundObjectMeta);
    [[nodiscard]] QString CreateRule(RuleObject);
    bool IsNodeConstructing() const
    {
        return isOperationLocked;
    }
    void LockOperation()
    {
        isOperationLocked = true;
    }

  public:
    const inline QStringList GetInboundTags() const
    {
        return inbounds.keys();
    }
    const inline QStringList GetOutboundTags() const
    {
        return outbounds.keys();
    }
    const inline QStringList GetRuleTags() const
    {
        return rules.keys();
    }
    inline int InboundsCount() const
    {
        return inbounds.count();
    }
    inline int RulesCount() const
    {
        return rules.count();
    }
    inline int OutboundsCount() const
    {
        return outbounds.count();
    }
    inline int ChainedOutboundsCount() const
    {
        return chainedOutboundNodes.count();
    }

  public:
    void OnNodeDeleted(const QtNodes::Node &node);
    void RequestEditChain(const ChainId &id);

    template<ComplexTagNodeMode t>
    inline bool RenameTag(const QString originalTag, const QString newTag)
    {
#define PROCESS(type)                                                                                                                           \
    bool hasExisting = type##s.contains(newTag);                                                                                                \
    if (hasExisting)                                                                                                                            \
        return false;                                                                                                                           \
    type##s[newTag] = type##s.take(originalTag);                                                                                                \
    type##Nodes[newTag] = type##Nodes.take(originalTag);

        if constexpr (t == NODE_INBOUND)
        {
            PROCESS(inbound);
        }
        else if constexpr (t == NODE_OUTBOUND)
        {
            PROCESS(outbound)
            if (chainedOutboundNodes.contains(originalTag))
                chainedOutboundNodes[newTag] = chainedOutboundNodes.take(originalTag);
        }
        else if constexpr (t == NODE_RULE)
        {
            PROCESS(rule)
        }
        else
        {
            Q_UNREACHABLE();
        }
        emit OnObjectTagChanged(t, originalTag, newTag);
        return true;
#undef PROCESS
    }

  signals:
    void OnInboundCreated(std::shared_ptr<INBOUND>, QtNodes::Node &);
    //
    void OnOutboundCreated(std::shared_ptr<OutboundObjectMeta>, QtNodes::Node &);
    void OnOutboundDeleted(const OutboundObjectMeta &object);
    //
    void OnRuleCreated(std::shared_ptr<RuleObject>, QtNodes::Node &);
    //
    void OnChainedOutboundCreated(std::shared_ptr<OutboundObjectMeta>, QtNodes::Node &);
    void OnChainedOutboundDeleted(const OutboundObjectMeta &object);
    //
    void OnObjectTagChanged(ComplexTagNodeMode, const QString originalTag, const QString newTag);

  signals:
    void OnInboundOutboundNodeHovered(const QString &tag, const ProtocolSettingsInfoObject &);

  private:
    void RestoreConnections();

  private:
    QString defaultOutbound;
    QMap<QString, QUuid> inboundNodes;
    QMap<QString, QUuid> outboundNodes;
    QMap<QString, QUuid> chainedOutboundNodes;
    QMap<QString, QUuid> ruleNodes;
    //
    QtNodes::FlowScene *ruleScene;
    QtNodes::FlowScene *chainScene;
    //
    bool isOperationLocked;
    QMap<QString, std::shared_ptr<INBOUND>> inbounds;
    QMap<QString, std::shared_ptr<RuleObject>> rules;
    QMap<QString, std::shared_ptr<OutboundObjectMeta>> outbounds;
};