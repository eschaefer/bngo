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
  let shownClass = isCurrentTrack ? "" : "child";

  <div
    ref
    role="button"
    className={"bb " ++ bgColor ++ " flex hide-child items-center"}>
    <div className="mr3">
      <Thumbnail url={isVisible ? track.artwork_url : ""} />
    </div>
    <span className=shownClass>
      <PlayerControl
        isPlaying={isCurrentTrack && isPlayerPlaying}
        handleClick=onTrackClick
      />
    </span>
    <div>
      <p> {React.string(track.title)} </p>
      <p> {React.string("Bumped by " ++ track.bumper)} </p>
    </div>
  </div>;
};