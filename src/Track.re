type track = {
  id: int,
  title: string,
  artwork_url: string,
  stream_url: string,
  bumper: string,
};

[@react.component]
let make = (~onTogglePlay, ~onPlay, ~track, ~currentTrack, ~isPlayerPlaying) => {
  let (isVisible, ref) = ReactIsVisible.useIsVisible();
  let isCurrentTrack = currentTrack.id == track.id;
  let onTrackClick = isCurrentTrack ? onTogglePlay : (_ => onPlay(track));
  let bgColor = isCurrentTrack ? "bg-washed-green" : "bg-washed-yellow";
  let shownClass = isCurrentTrack ? "" : "child dn db-l";

  <div
    ref
    role="button"
    className={"bb " ++ bgColor ++ " flex hide-child items-center"}>
    <Thumbnail url={isVisible ? track.artwork_url : ""} />
    <div className=shownClass>
      <PlayerControl
        isPlaying={isCurrentTrack && isPlayerPlaying}
        handleClick=onTrackClick
      />
    </div>
    <div className="db dn-l">
      <PlayerControl
        isPlaying={isCurrentTrack && isPlayerPlaying}
        handleClick=onTrackClick
      />
    </div>
    <div>
      <p> {React.string(track.title)} </p>
      <p> {React.string("Bumped by " ++ track.bumper)} </p>
    </div>
  </div>;
};