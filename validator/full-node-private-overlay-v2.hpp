/*
    This file is part of TON Blockchain Library.

    TON Blockchain Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    TON Blockchain Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TON Blockchain Library.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "full-node.h"

namespace ton::validator::fullnode {

class FullNodePrivateOverlayV2 : public td::actor::Actor {
 public:
  void process_broadcast(PublicKeyHash src, ton_api::tonNode_blockBroadcast& query);
  void process_broadcast(PublicKeyHash src, ton_api::tonNode_blockBroadcastCompressed& query);
  void process_block_broadcast(PublicKeyHash src, ton_api::tonNode_Broadcast& query);

  void process_broadcast(PublicKeyHash src, ton_api::tonNode_newShardBlockBroadcast& query);

  void process_broadcast(PublicKeyHash src, ton_api::tonNode_newBlockCandidateBroadcast &query);
  void process_broadcast(PublicKeyHash src, ton_api::tonNode_newBlockCandidateBroadcastCompressed &query);
  void process_block_candidate_broadcast(PublicKeyHash src, ton_api::tonNode_Broadcast &query);

  template <class T>
  void process_broadcast(PublicKeyHash, T&) {
    VLOG(FULL_NODE_WARNING) << "dropping unknown broadcast";
  }
  void receive_broadcast(PublicKeyHash src, td::BufferSlice query);

  void send_shard_block_info(BlockIdExt block_id, CatchainSeqno cc_seqno, td::BufferSlice data);
  void send_broadcast(BlockBroadcast broadcast);
  void send_block_candidate(BlockIdExt block_id, CatchainSeqno cc_seqno, td::uint32 validator_set_hash,
                            td::BufferSlice data);

  void start_up() override;
  void tear_down() override;

  void destroy() {
    stop();
  }

  FullNodePrivateOverlayV2(adnl::AdnlNodeIdShort local_id, ShardIdFull shard, std::vector<adnl::AdnlNodeIdShort> nodes,
                           std::vector<adnl::AdnlNodeIdShort> senders, FileHash zero_state_file_hash,
                           td::actor::ActorId<keyring::Keyring> keyring, td::actor::ActorId<adnl::Adnl> adnl,
                           td::actor::ActorId<rldp::Rldp> rldp, td::actor::ActorId<rldp2::Rldp> rldp2,
                           td::actor::ActorId<overlay::Overlays> overlays,
                           td::actor::ActorId<ValidatorManagerInterface> validator_manager,
                           td::actor::ActorId<FullNode> full_node)
      : local_id_(local_id)
      , shard_(shard)
      , nodes_(std::move(nodes))
      , senders_(std::move(senders))
      , zero_state_file_hash_(zero_state_file_hash)
      , keyring_(keyring)
      , adnl_(adnl)
      , rldp_(rldp)
      , rldp2_(rldp2)
      , overlays_(overlays)
      , validator_manager_(validator_manager)
      , full_node_(full_node) {
  }

 private:
  adnl::AdnlNodeIdShort local_id_;
  ShardIdFull shard_;
  std::vector<adnl::AdnlNodeIdShort> nodes_;
  std::vector<adnl::AdnlNodeIdShort> senders_;
  FileHash zero_state_file_hash_;

  td::actor::ActorId<keyring::Keyring> keyring_;
  td::actor::ActorId<adnl::Adnl> adnl_;
  td::actor::ActorId<rldp::Rldp> rldp_;
  td::actor::ActorId<rldp2::Rldp> rldp2_;
  td::actor::ActorId<overlay::Overlays> overlays_;
  td::actor::ActorId<ValidatorManagerInterface> validator_manager_;
  td::actor::ActorId<FullNode> full_node_;

  bool inited_ = false;
  overlay::OverlayIdFull overlay_id_full_;
  overlay::OverlayIdShort overlay_id_;
  UnixTime created_at_ = (UnixTime)td::Clocks::system();

  void try_init();
  void init();
  void get_stats_extra(td::Promise<std::string> promise);
};

class FullNodePrivateBlockOverlaysV2 {
 public:
  td::actor::ActorId<FullNodePrivateOverlayV2> choose_overlay(ShardIdFull shard);
  void update_overlays(td::Ref<MasterchainState> state, std::set<adnl::AdnlNodeIdShort> my_adnl_ids,
                       const FileHash& zero_state_file_hash, const td::actor::ActorId<keyring::Keyring>& keyring,
                       const td::actor::ActorId<adnl::Adnl>& adnl, const td::actor::ActorId<rldp::Rldp>& rldp,
                       const td::actor::ActorId<rldp2::Rldp>& rldp2,
                       const td::actor::ActorId<overlay::Overlays>& overlays,
                       const td::actor::ActorId<ValidatorManagerInterface>& validator_manager,
                       const td::actor::ActorId<FullNode>& full_node);
  void destroy_overlays();

 private:
  struct Overlays {
    struct ShardOverlay {
      td::actor::ActorOwn<FullNodePrivateOverlayV2> overlay_;
      std::vector<adnl::AdnlNodeIdShort> nodes_, senders_;
      bool is_sender_ = false;
    };
    std::map<ShardIdFull, ShardOverlay> overlays_;
  };

  std::map<adnl::AdnlNodeIdShort, Overlays> id_to_overlays_;  // local_id -> overlays
};

}  // namespace ton::validator::fullnode
