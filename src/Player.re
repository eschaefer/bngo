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

  let buttonDisplay = currentTrack.id == 0 ? "dn " : "";

  <>
    <div
      className="bb bg-washed-blue f3 flex z-999 justify-between pr3"
      style={ReactDOMRe.Style.make(~position="sticky", ~top="0", ())}>
      <div className="flex">
        <div className="mr3">
          <Thumbnail url={currentTrack.artwork_url} />
        </div>
        <div>
          <p> {ReasonReact.string(currentTrack.title)} </p>
          <button
            onClick={_ => currentTrack.id == 0 ? () : onTogglePlay()}
            className={
              buttonDisplay
              ++ "bn f6 dim ph3 pv2 mb2 outline-0 white bg-black pointer"
            }>
            {isPlayerPlaying
               ? ReasonReact.string("Pause") : ReasonReact.string("Play")}
          </button>
        </div>
      </div>
      <div className="mw2 mt3 self-start">
        <a href="/"> <img src=logo alt="Github logo" /> </a>
      </div>
    </div>
    <audio ref={ReactDOMRe.Ref.domRef(audioRef)} src=audioUrl />
  </>;
};