type track = {
  id: int,
  title: string,
  artwork_url: string,
  stream_url: string,
  bumper: string,
  bumperLink: string,
};

[@react.component]
let make =
    (
      ~onTogglePlay,
      ~setUsername,
      ~onPlay,
      ~track,
      ~currentTrack,
      ~isPlayerPlaying,
    ) => {
  let (isVisible, ref) = ReactIsVisible.useIsVisible();
  let isCurrentTrack = currentTrack.id == track.id;
  let onTrackClick = isCurrentTrack ? onTogglePlay : (_ => onPlay(track));
  let bgColor = isCurrentTrack ? "bg-blue" : "bg-dark-pink";
  let shownClass = isCurrentTrack ? "dn db-l" : "child dn db-l";

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
      <p className="washed-blue b"> {React.string(track.title)} </p>
      <p>
        {React.string("Bumped by ")}
        <a
          href="#"
          onClick={event => {
            ReactEvent.Mouse.preventDefault(event);
            setUsername(track.bumperLink);
          }}
          className="link underline black hover-white pointer">
          {React.string(track.bumper)}
        </a>
      </p>
    </div>
  </div>;
};