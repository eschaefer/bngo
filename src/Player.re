[@bs.module "./images/github.svg"] external logo: string = "default";

[@react.component]
let make =
    (
      ~currentTrack: Track.track,
      ~onTogglePlay,
      ~onPlayNext,
      ~isPlayerPlaying,
      ~clientId,
    ) => {
  let audioRef = React.useRef(Js.Nullable.null);

  React.useEffect2(
    () => {
      switch (audioRef |> React.Ref.current |> Js.Nullable.toOption) {
      | Some(element) =>
        if (isPlayerPlaying) {
          ReactDOMRe.domElementToObj(element)##play();
        } else {
          ReactDOMRe.domElementToObj(element)##pause();
        }
      | None => ()
      };

      None;
    },
    (isPlayerPlaying, currentTrack.id),
  );

  React.useEffect0(() => {
    switch (audioRef |> React.Ref.current |> Js.Nullable.toOption) {
    | Some(el) => Webapi.Dom.Element.addEventListener("ended", onPlayNext, el)
    | None => ()
    };

    None;
  });

  let audioUrl =
    currentTrack.id == 0
      ? ""
      : "https://api.soundcloud.com/tracks/"
        ++ string_of_int(currentTrack.id)
        ++ "/stream?client_id="
        ++ clientId;

  let hidden = currentTrack.id == 0 ? "dn " : "";

  <>
    <div
      className="bb bg-washed-blue f3 flex z-9999 justify-between pr3"
      style={ReactDOMRe.Style.make(~position="sticky", ~top="0", ())}>
      <div className="flex items-center">
        <Thumbnail url={currentTrack.artwork_url} />
        <span className=hidden>
          <PlayerControl
            isPlaying=isPlayerPlaying
            handleClick={_ => currentTrack.id == 0 ? () : onTogglePlay()}
          />
        </span>
        <p> {ReasonReact.string(currentTrack.title)} </p>
      </div>
      <div className="mw2 mt3 self-start">
        <a href="https://github.com/eschaefer/bngo" title="Edit on Github">
          <img src=logo alt="Github logo" />
        </a>
      </div>
    </div>
    <audio ref={ReactDOMRe.Ref.domRef(audioRef)} src=audioUrl />
  </>;
};